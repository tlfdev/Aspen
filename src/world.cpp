#include "world.h"

#include "ComponentFactory.h"
#include "calloutManager.h"
#include "channel.h"
#include "com_gen.h"
#include "command.h"
#include "component.h"
#include "componentMeta.hpp"
#include "delayedEvent.h"
#include "event.h"
#include "eventManager.h"
#include "log.h"
#include "objectManager.h"
#include "option.h"
#include "optionManager.h"
#include "player.h"
#include "serializer.h"
#include "socket.h"
#include "utils.h"
#include "zone.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <dirent.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

World* World::_ptr;
World* World::GetPtr()
{
    if (!World::_ptr)
    {
        World::_ptr = new World();
    }
    return World::_ptr;
}

World::World()
{
    _running = true;
    _chanid = 1;
    _server = nullptr;
    _motd = nullptr;
    _banner = nullptr;
    _updates = 0;
    _totalUpdateTime = 0;
    _totalSleepTime = 0;
    _commands = 0;
    _commandElapsed = 0;
    // events
    events.RegisterEvent("LivingPulse");
    events.RegisterEvent("WorldPulse");
    events.RegisterEvent("PlayerConnect");
    events.RegisterEvent("PlayerDisconnect");
    events.RegisterEvent("PlayerCreated");
    events.RegisterEvent("PlayerDeleted");
    events.RegisterEvent("Shutdown");
    events.RegisterEvent("Copyover");
    events.RegisterEvent("ObjectLoaded");
    events.RegisterEvent("ObjectDestroyed");
}
World::~World()
{
    if (_motd)
    {
        delete[] _motd;
    }
    if (_banner)
    {
        delete[] _banner;
    }

    for (auto cit : _channels)
    {
        delete cit.second;
    }

    for (auto cit : _state)
    {
        delete cit.second;
    }

    for (Zone* zone : _zones)
    {
        delete zone;
    }

    delete _server;
}

void World::InitializeServer()
{
    _server = new Server();
}

void World::Shutdown()
{
    _pmanager.Shutdown();
    SaveState();
    events.CallEvent("Shutdown", nullptr, static_cast<void*>(this));
    _running = false;
}

void World::Copyover(Player* mobile)
{
    std::list<Player*>* _users;
    int ruptime = (int)GetRealUptime();

    FILE* copyover = nullptr;
    char buff[16];

    copyover = fopen(COPYOVER_FILE, "wb");
    if (copyover == nullptr)
    {
        mobile->Message(MSG_ERROR, "couldn't open the copyover file.\nCopyover will not continue.");
        return;
    }

    fprintf(copyover, "%d\n", ruptime);
    sockaddr_in* addr = nullptr;
    // itterate through the players and write info to their copyover file:
    _users = _pmanager.GetPlayers();
    for (auto person : *_users)
    {
        if (person->GetSocket()->GetConnectionType() != CON_Game)
        {
            person->Write("We're sorry, but we are currently rebooting; please come back again soon.\n");
            person->GetSocket()->Kill();
            continue;
        }

        addr = person->GetSocket()->GetAddr();
        person->Save();
        fprintf(copyover, "%d %s %hu %lu %s\n", person->GetSocket()->GetControl(), person->GetName().c_str(),
                addr->sin_port, (long int)addr->sin_addr.s_addr, person->GetSocket()->GetHost().c_str());
        person->Write("Copyover initiated by " + mobile->GetName() + ".\n");
    }

    fprintf(copyover, "-1\n");
    fclose(copyover);

    events.CallEvent("Copyover", nullptr, static_cast<void*>(this));
    snprintf(buff, 16, "%d", _server->GetListener());

    Update();
    SaveState();
    execl(BIN_FILE, BIN_FILE, "-c", buff, (char*)nullptr);

    mobile->Write("Copyover failed!\n");
}

Server* World::GetServer() const
{
    return _server;
}

OlcManager* World::GetOlcManager()
{
    return &_olcs;
}

ComponentFactory* World::GetComponentFactory()
{
    return &_cfactory;
}

PlayerManager& World::GetPlayerManager()
{
    return _pmanager;
}

OptionManager* World::GetOptionManager()
{
    return &_options;
}

void World::GetChannelNames(std::list<std::string>* out)
{
    for (auto it : _channels)
    {
        out->push_back(it.second->GetName());
    }
}

bool World::ChannelExists(Channel* chan)
{
    for (auto it : _channels)
    {
        if (it.second == chan)
        {
            return true;
        }
    }

    return false;
}
bool World::AddChannel(Channel* chan, bool command)
{
    OptionMeta* opt = nullptr;
    if (!ChannelExists(chan))
    {
        _channels[_chanid] = chan;
        opt = new OptionMeta();
        opt->SetName(chan->GetName());
        opt->SetHelp("Toggles the channel.");
        opt->SetToggle(true);
        opt->SetSection(OptionSection::Channel);
        opt->SetAccess(chan->GetAccess());
        if (chan->GetName() == "newbie")
        {
            opt->SetValue(Variant(1));
        }
        else
        {
            opt->SetValue(Variant(0));
        }
        _options.AddOption(opt);

        if (command)
        {
            CMDChan* com = new CMDChan();
            com->SetName(chan->GetName());
            com->SetAccess(chan->GetAccess());
            com->SetSubcmd(_chanid);
            if (chan->GetAlias() != "")
            {
                com->AddAlias(chan->GetAlias());
            }

            commands.AddCommand(com);
        }

        _chanid++;
        return true;
    }

    return false;
}

Channel* World::FindChannel(int id)
{
    if (!_channels.count(id))
    {
        return nullptr;
    }

    return _channels[id];
}

Channel* World::FindChannel(const std::string& name)
{
    // This method is a bit slower because we have to iterate through the mapping ourselves.

    for (auto it : _channels)
    {
        if ((it.second)->GetName() == name)
        {
            return (it.second);
        }
    }

    return nullptr;
}

bool World::InitializeFiles()
{
    struct stat fs; // holds file stats
    // load our banner:
    // retrieve size of file so we can create the buffer:
    if (stat(LOGIN_FILE, &fs))
    {
        WriteLog(SeverityLevel::Fatal, "Could not stat login file.");
        return false;
    }

    _banner = new char[fs.st_size + 1];
    _banner[fs.st_size] = '\0';

    // open and load the banner:
    FILE* banner_fd = fopen(LOGIN_FILE, "r");
    if (!banner_fd)
    {
        WriteLog(SeverityLevel::Fatal, "Could not fopen banner file.");
        delete[] _banner;
        _banner = nullptr;
        return false;
    }
    if (fread(_banner, 1, static_cast<size_t>(fs.st_size), banner_fd) != static_cast<size_t>(fs.st_size))
    {
        WriteLog("SeverityLevel::Fatal, Error loading banner.");
        delete[] _banner;
        _banner = nullptr;
        fclose(banner_fd);
        return false;
    }
    fclose(banner_fd);

    // load our motd:
    // retrieve size of file so we can create the buffer:
    if (stat(MOTD_FILE, &fs))
    {
        WriteLog(SeverityLevel::Fatal, "Could not stat MOTD file.");
        delete[] _banner;
        _banner = nullptr;
        return false;
    }

    _motd = new char[fs.st_size + 1];
    _motd[fs.st_size] = '\0';

    FILE* motd_fd = fopen(MOTD_FILE, "r");
    if (!motd_fd)
    {
        WriteLog(SeverityLevel::Fatal, "Could not fopen MOTD.");
        delete[] _banner;
        delete[] _motd;
        _motd = _banner = nullptr;
        return false;
    }

    if (fread(_motd, 1, static_cast<size_t>(fs.st_size), motd_fd) != static_cast<size_t>(fs.st_size))
    {
        WriteLog(SeverityLevel::Fatal, "Error loading MOTD.");
        delete[] _motd;
        _banner = nullptr;
        fclose(motd_fd);
        return false;
    }
    fclose(motd_fd);
    WriteLog("Files loaded successfully");
    return true;
}

const char* World::GetBanner() const
{
    return _banner;
}
const char* World::GetMotd() const
{
    return _motd;
}

void World::UpdateZones()
{
    for (auto zone : _zones)
    {
        zone->Update();
    }
}

void World::Update()
{
    timeval start, end;
    gettimeofday(&start, nullptr);
    // checks for incoming connections or commands
    _server->PollSockets();
    // flushes the output buffers of all sockets.
    _server->FlushSockets();
    // update living objects:
    _pmanager.Update();
    UpdateZones();
    _objectManager.Update();
    CalloutManager* callouts = CalloutManager::GetInstance();
    callouts->Update();

    _updates++;
    gettimeofday(&end, nullptr);
    _totalUpdateTime += ((end.tv_sec - start.tv_sec) * 1000000);
    _totalUpdateTime += (end.tv_usec - start.tv_usec);

    // sleep so that we don't kill our cpu
    _totalSleepTime += _server->Sleep(PULSES_PER_SECOND);
}

bool World::RegisterComponent(IComponentMeta* meta)
{
    return _cfactory.RegisterComponent(meta->GetName(), meta);
}
Component* World::CreateComponent(const std::string& name)
{
    return _cfactory.Create(name);
}

time_t World::GetRealUptime() const
{
    return _ruptime;
}
void World::SetRealUptime(time_t tm)
{
    _ruptime = tm;
}

time_t World::GetCopyoverUptime() const
{
    return _cuptime;
}
void World::SetCopyoverUptime(time_t tm)
{
    _cuptime = tm;
}

bool World::AddProperty(const std::string& name, void* ptr)
{
    if (!_properties.count(name))
    {
        _properties[name] = ptr;
        return true;
    }

    return false;
}
void* World::GetProperty(const std::string& name)
{
    if (_properties.count(name))
    {
        return _properties[name];
    }

    return nullptr;
}
bool World::RemoveProperty(const std::string& name)
{
    if (_properties.count(name))
    {
        _properties.erase(name);
        return true;
    }

    return false;
}
void World::ParseArguments(const std::string& args, int start, std::vector<std::string>& params)
{
    if (start < 0)
        start = 0;
    std::string_view sv(args);
    if (static_cast<size_t>(start) >= sv.size())
        return;

    size_t i = static_cast<size_t>(start);
    const size_t n = sv.size();

    while (i < n)
    {
        // skip spaces
        while (i < n && sv[i] == ' ')
            ++i;
        if (i >= n)
            break;

        // quoted argument?
        if (sv[i] == '"' || sv[i] == '\'')
        {
            const char quote = sv[i++];
            const size_t begin = i;

            // find closing quote (or end of string)
            while (i < n && sv[i] != quote)
                ++i;

            // push [begin, i)
            params.emplace_back(std::string(sv.substr(begin, i - begin)));

            // if we stopped on a quote, skip it
            if (i < n && sv[i] == quote)
                ++i;

            // continue to next token
            continue;
        }

        // unquoted token: read until next space
        const size_t begin = i;
        while (i < n && sv[i] != ' ')
            ++i;

        params.emplace_back(std::string(sv.substr(begin, i - begin)));
        // loop continues; trailing space is skipped by the next iteration
    }
}

bool World::AddZone(Zone* zone)
{
    if (_zones.size())
    {
        for (auto it : _zones)
        {
            if (it == zone)
            {
                return false;
            }
        }
    }

    _zones.push_back(zone);
    return true;
}
bool World::RemoveZone(Zone* zone)
{
    std::vector<Zone*>::iterator it, itEnd;

    itEnd = _zones.end();
    for (it = _zones.begin(); it != itEnd; ++it)
    {
        if (*it == zone)
        {
            _zones.erase(it);
            return true;
        }
    }

    return false;
}
Zone* World::GetZone(const std::string& name)
{
    for (auto it : _zones)
    {
        if (name == it->GetName())
        {
            return it;
        }
    }

    return nullptr;
}
bool World::GetZones(std::vector<Zone*>* zones)
{
    std::copy(_zones.begin(), _zones.end(), std::back_inserter(*zones));
    return true;
}

bool World::IsRunning() const
{
    return _running;
}
void World::SetRunning(bool running)
{
    _running = running;
}

bool World::PromptExists(char prompt)
{
    return (_prompts.count(prompt) == 0 ? false : true);
}
bool World::RegisterPrompt(char c, PROMPTCB callback)
{
    if (PromptExists(c))
    {
        return false;
    }

    _prompts[c] = callback;
    return true;
}
std::string World::BuildPrompt(const std::string& prompt, Player* mobile)
{
    std::string::const_iterator it, itEnd;
    std::string ret;

    itEnd = prompt.end();
    for (it = prompt.begin(); it != itEnd; ++it)
    {
        if ((*it) == '%' && ++it != itEnd)
        {
            if (PromptExists((*it)))
            {
                ret += (_prompts[(*it)])(mobile);
            }
            else
            {
                ret += '%';

                ret += (*it);
            }
        }
        else
        {
            ret += (*it);
        }
    }

    return ret;
}

bool World::AddState(const std::string& name, ISerializable* s)
{
    if (StateExists(name))
    {
        return false;
    }

    _state[name] = s;
    return true;
}
bool World::RemoveState(const std::string& name)
{
    if (!StateExists(name))
    {
        return false;
    }

    _state.erase(name);
    return true;
}
bool World::StateExists(const std::string& name)
{
    return (_state.count(name) == 1 ? true : false);
}

bool World::SaveState()
{
    for (auto sit : _state)
    {
        std::string filename = STATE_DIR + sit.first + ".json";
        if (!JsonFileSerializer::SaveToFile(filename, *(sit.second), "State"))
        {
            WriteLog(SeverityLevel::Warning, "Could not save state file: " + filename);
        }
    }

    return true;
}
bool World::LoadState()
{
    DIR* statedir = opendir(STATE_DIR);
    dirent* dir = nullptr;

    // We need to open the directory for reading
    if (!statedir)
    {
        return false;
    }

    while ((dir = readdir(statedir)))
    {
        if (dir->d_name[0] == '.')
        {
            continue;
        }

        std::string filename = dir->d_name;
        std::string filepath = std::string(STATE_DIR) + filename;

        // Extract state name from filename (remove .json extension if present)
        std::string name = filename;
        size_t dotPos = name.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            name = name.substr(0, dotPos);
        }

        if (!StateExists(name))
        {
            WriteLog(SeverityLevel::Warning, "Could not find a matching registered state for " + name +
                                                 " in the state register. This state will not be deserialized.");
            continue;
        }

        if (!JsonFileSerializer::LoadFromFile(filepath, *(_state[name]), "State"))
        {
            WriteLog(SeverityLevel::Warning, "Could not load " + filename + " state file.");
        }
    }

    closedir(statedir);
    return true;
}

unsigned long long int World::GetUpdates() const
{
    return _updates;
}
unsigned long long int World::GetUpdateTime() const
{
    return _totalUpdateTime;
}
unsigned long long int World::GetSleepTime() const
{
    return _totalSleepTime;
}

unsigned long long int World::GetCommands() const
{
    return _commands;
}
unsigned long long int World::GetCommandTime() const
{
    return _commandElapsed;
}

ObjectManager* World::GetObjectManager()
{
    return &_objectManager;
}

bool [[nodiscard]] World::DoCommand(Player* mobile, const std::string args)
{
    using clock = std::chrono::steady_clock;

    if (args.empty())
        return false;

    // Input sanitization: limit command length to prevent DoS
    constexpr size_t MAX_COMMAND_LENGTH = 8192;
    if (args.length() > MAX_COMMAND_LENGTH)
    {
        mobile->Message(MSG_ERROR, "Command too long.");
        return false;
    }

    const auto t0 = clock::now();

    const std::vector<Command*>* cptr = commands.GetPtr();

    std::string_view line(args);
    std::string cmd;
    size_t i = 0;

    // special prefixes
    const char first = line.front();
    if (first == '"' || first == '\'')
    {
        cmd = "say";
        i = 1; // arguments start after the quote
    }
    else if (first == ':')
    {
        cmd = "emote";
        i = 1;
    }
    else [[likely]]
    {
        // verb is up to first space (or whole string)
        const size_t sp = line.find(' ');
        if (sp == std::string_view::npos)
        {
            cmd.assign(line.begin(), line.end());
            i = line.size();
        }
        else
        {
            cmd.assign(line.begin(), line.begin() + static_cast<std::ptrdiff_t>(sp));
            i = sp;
        }
    }

    // parse arguments if any
    std::vector<std::string> params;
    if (i < line.size())
        ParseArguments(args, static_cast<int>(i), params);

    // dispatch: built-ins first (inventory/room can mirror this block)
    for (Command* c : *cptr)
    {
        // keep semantics: exact name or alias match
        if (c->GetName() == cmd || c->HasAlias(cmd, true))
        {
            if (!mobile->HasAccess(c->GetAccess()))
                return false;

            // execute
            c->Execute(c->GetName(), mobile, params, c->GetSubcmd());

            const auto t1 = clock::now();
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            _commandElapsed += us;
            _commands += 1;
            return true;
        }
    }

    // TODO: check inventory and room commands similarly
    return false;
}
