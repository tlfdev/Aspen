#include "player.h"

#include "calloutManager.h"
#include "conf.h"
#include "event.h"
#include "eventargs.h"
#include "exception.h"
#include "mud.h"
#include "objectManager.h"
#include "olc.h"
#include "option.h"
#include "optionManager.h"
#include "optionMeta.h"
#include "utils.h"
#include "world.h"

#include <tinyxml2.h>

#include <cmath>
#include <cstdarg>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

Player::Player()
{
    _invalidPassword = 0;
    _prompt = ">";
    _title = "the brave";
    _rank = RANK_PLAYER;
    _pflag = 0;
    _firstLogin = 0;
    _onlineTime = 0;
    _lastLogin = 0;
    _lastSave = 0;
    _lastBackup = 0;
    _watching = nullptr;
    _watchers = new std::list<Player*>();

    // messages:
    _messages = new std::map<MessageType, std::string>();
    AddMessage(MSG_ERROR, C_RED);
    AddMessage(MSG_INFO, C_BLUE);
    AddMessage(MSG_CRITICAL, CB_CYAN);
    AddMessage(MSG_CHANNEL, C_YELLOW);
    AddMessage(MSG_LIST, C_CYAN);

    // events
    events.RegisterEvent("EnterGame");
    events.RegisterEvent("LeaveGame");
    events.RegisterEvent("OptionChanged");
}
Player::~Player()
{
    if (_messages)
    {
        delete _messages;
        _messages = nullptr;
    }

    for (Option* oit : _config)
    {
        delete oit;
    }
    delete _watchers;
}

bool Player::IsPlayer() const
{
    return true;
}
// JSON serialization
void Player::ToJson(Json::Value& json) const
{
    using namespace JsonSerializerHelpers;

    // Serialize base living
    Living::ToJson(json);

    // Serialize Player-specific fields
    json["password"] = _password;
    json["invalidPassword"] = _invalidPassword;
    json["title"] = _title;
    json["prompt"] = _prompt;
    json["rank"] = _rank;
    json["pflag"] = _pflag;
    json["firstLogin"] = static_cast<unsigned int>(_firstLogin);
    json["onlineTime"] = static_cast<unsigned int>(_onlineTime);
    json["lastLogin"] = static_cast<unsigned int>(_lastLogin);

    // Serialize options
    Json::Value optionsArray(Json::arrayValue);
    for (const auto* opt : _config)
    {
        if (opt && opt->GetMeta())
        {
            Json::Value optJson;
            optJson["name"] = opt->GetMeta()->GetName();
            opt->GetValue().ToJson(optJson);
            optionsArray.append(optJson);
        }
    }
    json["options"] = optionsArray;
}

void Player::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    // Deserialize base living
    Living::FromJson(json, version);

    // Deserialize Player-specific fields
    _password = GetString(json, "password", "");
    _invalidPassword = GetInt(json, "invalidPassword", 0);
    _title = GetString(json, "title", "the brave");
    _prompt = GetString(json, "prompt", ">");
    _rank = GetUInt(json, "rank", RANK_PLAYER);
    _pflag = GetUInt(json, "pflag", 0);
    _firstLogin = GetUInt(json, "firstLogin", 0);
    _onlineTime = GetUInt(json, "onlineTime", 0);
    _lastLogin = GetUInt(json, "lastLogin", 0);

    // Deserialize options
    if (json.isMember("options") && json["options"].isArray())
    {
        World* world = World::GetPtr();
        OptionManager* omanager = world->GetOptionManager();
        const Json::Value& optionsArray = json["options"];

        for (const auto& optJson : optionsArray)
        {
            if (optJson.isMember("name"))
            {
                std::string name = optJson["name"].asString();
                OptionMeta* ometa = omanager->GetOption(name);
                if (ometa && optJson.isMember("variable"))
                {
                    Variant var;
                    var.FromJson(optJson["variable"], version);
                    Option* opt = new Option(ometa, var);
                    _config.push_back(opt);
                }
            }
        }
    }
}

void Player::SetSocket(Socket* sock)
{
    _sock = sock;
}
std::string Player::GetShort() const
{
    std::string ret = GetName() + " " + GetTitle();
    ret += ".";
    return ret;
}

std::string Player::GetPassword() const
{
    return _password;
}
void Player::SetPassword(const std::string& s)
{
    _password = Sha256Hash(s);
}

std::string Player::GetTempPassword() const
{
    return _tempPassword;
}
void Player::SetTempPassword(const std::string& s)
{
    _tempPassword = Sha256Hash(s);
}
void Player::ClearTempPassword()
{
    _tempPassword.clear();
}
bool Player::ComparePassword()
{
    return _password == _tempPassword;
}

void Player::IncInvalidPassword()
{
    _invalidPassword++;
    Save(true);
}

bool Player::Save(bool force)
{
    if (!force)
    {
        if ((time(nullptr) - _lastSave) < SAVE_INTERVAL)
        {
            return false;
        }
        _lastSave = time(nullptr);
    }

    std::string path = std::string(PLAYER_DIR) + GetName() + ".json";
    return JsonFileSerializer::SaveToFile(path, *this, "Player");
}

bool Player::Backup()
{
    if ((time(nullptr) - _lastBackup) < BACKUP_INTERVAL)
    {
        return false;
    }
    /**
     * @todo update to make save take a backup param.
     */
    return true;
}

void Player::Load()
{
    std::string path = std::string(PLAYER_DIR) + GetName() + ".json";
    if (!JsonFileSerializer::LoadFromFile(path, *this, "Player"))
    {
        throw(FileLoadException("Error loading player: " + GetName()));
    }
}

void Player::EnterGame()
{
    EnterGame(false);
}
void Player::EnterGame(bool quiet)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    ObjectContainer* location = nullptr;
    location = GetLocation();
    if (location == nullptr)
    {
        MoveTo(omanager->GetRoom(ROOM_START));
        location = GetLocation();
    }
    else
    {
        location->ObjectEnter(this);
    }

    Living::EnterGame();
    // add the player to the users list:
    world->GetPlayerManager().AddPlayer(this);
    // if there were password attempts, tell the player.
    if (_invalidPassword)
    {
        std::stringstream st;
        st << (char)7 << _invalidPassword << (_invalidPassword == 1 ? " attempt was" : " attempts were")
           << " made on your password.";
        Message(MSG_CRITICAL, st.str());
        _invalidPassword = 0;
    }
    if (!quiet)
    {
        // show the login banner:
        Write("\n" + std::string(world->GetMotd()) + "\n");
    }
    world->events.CallEvent("PlayerConnect", nullptr, this);
    events.CallEvent("EnterGame", nullptr, this);
    Attach();
    Save();

    if (location->IsRoom())
    {
        Message(MSG_INFO, ((Room*)location)->DoLook(this));
    }
}
void Player::LeaveGame()
{
    World* world = World::GetPtr();

    Save(true);
    Living::LeaveGame();
    // take the player from the users list:
    world->GetPlayerManager().RemovePlayer(this);
    world->events.CallEvent("PlayerDisconnect", nullptr, this);
    events.CallEvent("LeaveGame", nullptr, this);
}

void Player::Write(const std::string& text) const
{
    if (_sock)
        _sock->Write(text);
}

void Player::Message(const MessageType type, const std::string& data) const
{
    Write((*_messages)[type] + data + C_NORMAL + "\n");
}

Socket* Player::GetSocket()
{
    return _sock;
}

std::string Player::GetTitle() const
{
    return _title;
}
void Player::SetTitle(const std::string& s)
{
    _title = s;
}

FLAG Player::GetRank() const
{
    return _rank;
}
void Player::SetRank(const FLAG s)
{
    _rank = s;
}
FLAG Player::GetPflag() const
{
    return _pflag;
}
void Player::SetPflag(FLAG flag)
{
    _pflag = flag;
}
unsigned int Player::GetOnlineTime() const
{
    return (unsigned int)_onlineTime;
}
void Player::SetOnlineTime(unsigned int s)
{
    _onlineTime = (time_t)s;
}

unsigned int Player::GetFirstLogin() const
{
    return (unsigned int)_firstLogin;
}
void Player::SetFirstLogin(unsigned int first)
{
    _firstLogin = (time_t)first;
}

unsigned int Player::GetLastLogin() const
{
    return (time_t)_lastLogin;
}
void Player::SetLastLogin(unsigned int last)
{
    _lastLogin = (time_t)last;
}

std::string Player::GetPrompt() const
{
    return _prompt;
}
void Player::SetPrompt(const std::string& prompt)
{
    _prompt = prompt;
}

void Player::SetOption(const std::string& option, Variant& val)
{
    Option* opt = nullptr;
    OptionMeta* ometa = nullptr;

    opt = GetOption(option);
    if (OptionExists(option) && opt)
    {
        if (opt->GetMeta()->GetValue().Typeof() == val.Typeof())
        {
            opt->GetValue().SetStr(val.GetStr());
        }
    }
    else
    {
        // checks for the global existance of the option.
        ometa = opt->GetMeta();
        if (ometa->GetValue().Typeof() == val.Typeof())
        {
            opt = new Option(ometa, val);
            _config.push_back(opt);
        }
    }
}
Option* Player::GetOption(const std::string& option) const
{
    for (Option* opt : _config)
    {
        if (opt->GetMeta()->GetName() == option)
        {
            return opt;
        }
    }

    return nullptr;
}

bool Player::OptionExists(const std::string& option) const
{
    for (Option* opt : _config)
    {
        if (opt->GetMeta()->GetName() == option)
        {
            return true;
        }
    }

    return false;
}
bool Player::ToggleOption(const std::string& option)
{
    int temp = 0;
    World* world = World::GetPtr();
    OptionManager* omanager = world->GetOptionManager();
    OptionMeta* ometa = nullptr;
    Option* node = nullptr;

    if ((omanager->OptionExists(option)) && (!OptionExists(option)))
    {
        ometa = omanager->GetOption(option);
        if (ometa->GetValue().Typeof() == VAR_INT)
        {
            if (ometa->GetValue().GetInt())
            {
                temp = 0;
            }
            else
            {
                temp = 1;
            }
            node = new Option(ometa, Variant(temp));
            _config.push_back(node);
            OptionChangedArgs arg(node);
            events.CallEvent("OptionChanged", &arg, this);
        }
        return true;
    }
    node = GetOption(option);
    if (node)
    {
        if (node->GetValue().Typeof() == VAR_INT)
        {
            if (node->GetValue().GetInt())
            {
                node->SetValue(Variant(0));
            }
            else
            {
                node->SetValue(Variant(1));
            }
            OptionChangedArgs arg(node);
            events.CallEvent("OptionChanged", &arg, this);
        }
        return true;
    }

    return false;
}
std::vector<Option*>* Player::GetOptions()
{
    return &_config;
}

bool Player::HasAccess(FLAG access) const
{
    return BitIsSet(_rank, access);
}
void Player::AddMessage(MessageType type, const std::string& color)
{
    (*_messages)[type] = color;
}

void InitializePlayer()
{
    CalloutManager* manager = CalloutManager::GetInstance();
    World* world = World::GetPtr();
    std::list<Player*>* players = world->GetPlayerManager().GetPlayers();

    manager->RegisterCallout(
        60 * 30, 0,
        [players](Callout* foo)
        {
            for (auto person : *players)
            {
                person->Save(true);
            }
        },
        false);
    manager->RegisterCallout(
        3, 0,
        [players](Callout* callout)
        {
            for (auto person : *players)
            {
                person->SetOnlineTime(person->GetOnlineTime() + 3);
            }
        },
        false);
}
