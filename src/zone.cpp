#include "zone.h"

#include "event.h"
#include "log.h"
#include "objectManager.h"
#include "room.h"
#include "serializationHelpers.h"
#include "staticObject.h"
#include "utils.h"
#include "world.h"

#include <tinyxml2.h>

#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include <dirent.h>

#include <sys/stat.h>

static int zone_saves;

Zone::Zone()
{
    _vnumrange.min = 0;
    _vnumrange.max = 0;
    _resetfreq = 240;
    _resetmsg = "With a pop, the area resets around you.";
    _lastreset = time(nullptr);
    _creation = time(nullptr);
    _opened = 0;
    _flags = 0;
}
Zone::~Zone()
{
    for (auto it : _virtualobjs)
    {
        delete it;
    }

    for (auto it : _objects)
    {
        delete it;
    }

    for (auto it : _roomobjs)
    {
        delete it;
    }

    for (auto it : _mobobjs)
    {
        delete it;
    }
}

std::string Zone::GetName() const
{
    return _name;
}
void Zone::SetName(const std::string& name)
{
    _name = name;
}
void Zone::SetRange(int min, int max)
{
    _vnumrange.min = min;
    _vnumrange.max = max;
}
int Zone::GetMinVnum()
{
    return _vnumrange.min;
}
int Zone::GetMaxVnum()
{
    return _vnumrange.max;
}
Room* Zone::AddRoom()
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    Room* room = nullptr;
    VNUM num = omanager->GetFreeRoomVnum(_vnumrange.min, _vnumrange.max);

    if (!num)
    {
        throw(std::runtime_error("No more vnums available"));
    }

    room = new Room();
    room->SetOnum(num);
    room->SetZone(this);
    _roomobjs.push_back(room);
    omanager->AddRoom(room);
    return room;
}
Room* Zone::AddRoom(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    Room* room = nullptr;
    if (!omanager->GetFreeRoomVnum(_vnumrange.min, _vnumrange.max, num))
    {
        throw(std::runtime_error("No more vnums available"));
    }

    room = new Room();
    room->SetOnum(num);
    room->SetZone(this);
    _roomobjs.push_back(room);
    omanager->AddRoom(room);
    return room;
}
bool Zone::RemoveRoom(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    std::vector<Room*>::iterator it, itEnd;

    if (omanager->RoomExists(num))
    {
        itEnd = _roomobjs.end();
        for (it = _roomobjs.begin(); it != itEnd; ++it)
        {
            if ((*it)->GetOnum() == num)
            {
                _roomobjs.erase(it);
                omanager->RemoveRoom(num);
                return true;
            }
        }
    }

    return false;
}
void Zone::GetRooms(std::vector<Room*>* rooms)
{
    std::copy(_roomobjs.begin(), _roomobjs.end(), std::back_inserter(*rooms));
}
bool Zone::RoomExists(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    if (num < _vnumrange.min || num > _vnumrange.max)
    {
        return false;
    }

    return omanager->RoomExists(num);
}

StaticObject* Zone::AddVirtual()
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    StaticObject* obj = nullptr;
    VNUM num = omanager->GetFreeVirtualVnum(_vnumrange.min, _vnumrange.max);

    if (!num)
    {
        throw(std::runtime_error("No more vnums available"));
    }

    obj = new StaticObject();
    obj->SetOnum(num);
    _virtualobjs.push_back(obj);
    omanager->AddVirtual(obj);
    return obj;
}
bool Zone::RemoveVirtual(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    std::vector<StaticObject*>::iterator it, itEnd;

    itEnd = _virtualobjs.end();
    for (it = _virtualobjs.begin(); it != itEnd; ++it)
    {
        if ((*it)->GetOnum() == num)
        {
            _virtualobjs.erase(it);
            omanager->RemoveVirtual(num);
            return true;
        }
    }

    return false;
}
StaticObject* Zone::GetVirtual(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();

    if (VirtualExists(num))
    {
        return omanager->GetVirtual(num);
    }

    return nullptr;
}
void Zone::GetVirtuals(std::vector<StaticObject*>* objects)
{
    std::copy(_virtualobjs.begin(), _virtualobjs.end(), std::back_inserter(*objects));
}
bool Zone::VirtualExists(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    if (num > _vnumrange.max || num < _vnumrange.min)
    {
        return false;
    }

    return omanager->VirtualExists(num);
}

Npc* Zone::AddNpc()
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    Npc* mob = nullptr;
    VNUM num = omanager->GetFreeNpcVnum(_vnumrange.min, _vnumrange.max);

    if (!num)
    {
        throw(std::runtime_error("No more vnums available"));
    }

    mob = new Npc();
    mob->SetOnum(num);
    _mobobjs.push_back(mob);
    omanager->AddNpc(mob);
    return mob;
}
bool Zone::RemoveNpc(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    std::vector<Npc*>::iterator it, itEnd;

    itEnd = _mobobjs.end();
    for (it = _mobobjs.begin(); it != itEnd; ++it)
    {
        if ((*it)->GetOnum() == num)
        {
            _mobobjs.erase(it);
            omanager->RemoveNpc(num);
            return true;
        }
    }

    return false;
}
Npc* Zone::GetNpc(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    if (NpcExists(num))
    {
        return omanager->GetNpc(num);
    }

    return nullptr;
}
void Zone::GetNpcs(std::vector<Npc*>* npcs)
{
    std::copy(_mobobjs.begin(), _mobobjs.end(), std::back_inserter(*npcs));
}
bool Zone::NpcExists(VNUM num)
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    if (num > _vnumrange.max || num < _vnumrange.min)
    {
        return false;
    }

    return omanager->NpcExists(num);
}
Npc* Zone::CreateNpc(VNUM num, Room* origin)
{
    Npc* templ = GetNpc(num);
    Npc* ret = nullptr;

    if (!templ)
    {
        return nullptr;
    }

    ret = new Npc();
    templ->Copy(ret);
    ret->SetOrigin(origin);
    ret->EnterGame();
    ret->Initialize();
    if (!ret->MoveTo(origin))
    {
        delete ret;
        return nullptr;
    }

    return ret;
}

Entity* Zone::CreateObject(VNUM num)
{
    const auto templ = GetVirtual(num);
    if (!templ)
    {
        return nullptr;
    }

    auto object = templ->Create();
    return object;
}

void Zone::Update() {}

// JSON serialization
void Zone::ToJson(Json::Value& json) const
{
    using namespace JsonSerializerHelpers;

    json["name"] = _name;
    json["flags"] = _flags;
    json["creation"] = static_cast<unsigned int>(_creation);
    json["opened"] = static_cast<unsigned int>(_opened);
    json["resetmsg"] = _resetmsg;
    json["resetfreq"] = _resetfreq;
    json["minvnum"] = _vnumrange.min;
    json["maxvnum"] = _vnumrange.max;

    // Serialize virtual objects
    SerializeVector(json, "virtualobjs", _virtualobjs);

    // Serialize rooms
    SerializeVector(json, "rooms", _roomobjs);

    // Serialize NPCs
    SerializeVector(json, "npcs", _mobobjs);
}

void Zone::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();

    _name = GetString(json, "name", "");
    _flags = GetInt(json, "flags", 0);
    _creation = GetUInt(json, "creation", 0);
    _opened = GetUInt(json, "opened", 0);
    _resetmsg = GetString(json, "resetmsg", "");
    _resetfreq = GetUInt(json, "resetfreq", 240);
    _vnumrange.min = GetInt(json, "minvnum", 0);
    _vnumrange.max = GetInt(json, "maxvnum", 0);

    // Deserialize virtual objects
    DeserializeVector(json, "virtualobjs", _virtualobjs, version);

    // Deserialize rooms
    DeserializeVector(json, "rooms", _roomobjs, version);

    // Deserialize NPCs
    DeserializeVector(json, "npcs", _mobobjs, version);

    // Register with managers
    for (auto* room : _roomobjs)
    {
        omanager->AddRoom(room);
        room->SetZone(this);
    }

    for (auto* vobj : _virtualobjs)
    {
        omanager->AddVirtual(vobj);
    }

    for (auto* npc : _mobobjs)
    {
        omanager->AddNpc(npc);
    }
}

bool InitializeZones()
{
    World* world = World::GetPtr();
    struct stat FInfo;

    WriteLog("Initializing areas.");
    if ((stat(AREA_STARTFILE, &FInfo)) != -1)
    {
        Zone::LoadZones();
    }
    else
    {
#ifdef NO_INIT_DEFAULTS
        WriteLog("No area file exists, and NO_INIT_DEFAULTS was enabled, exiting.");
        return false;
    }
#else
        WriteLog("No area found, creating default.");
        // no zones and rooms exist, create a first zone/room.
        Zone* zone = new Zone();
        if (!zone)
        {
            return false;
        }
        zone->SetName("Start");
        if (!world->AddZone(zone))
        {
            return false;
        }
        zone->SetRange(1, 100);
        Room* room = zone->AddRoom(ROOM_START);
        room->SetName("A blank room");
        if (!Zone::SaveZones())
        {
            return false;
        }
    }
#endif

    zone_saves = 0;
    world->events.AddCallback("WorldPulse", std::bind(&Zone::Autosave, std::placeholders::_1, std::placeholders::_2));
    world->events.AddCallback("Shutdown", std::bind(&Zone::Shutdown, std::placeholders::_1, std::placeholders::_2));
    world->events.AddCallback("Copyover", std::bind(&Zone::Shutdown, std::placeholders::_1, std::placeholders::_2));
    return true;
}

bool Zone::SaveZones()
{
    World* world = World::GetPtr();
    std::vector<Zone*>* zones = new std::vector<Zone*>();

    world->GetZones(zones);
    if (zones->size())
    {
        for (Zone* zone : *zones)
        {
            std::string path = std::string(AREA_DIR) + zone->GetName() + ".json";
            JsonFileSerializer::SaveToFile(path, *zone, "Zone");
        }
    }
    delete zones;
    return true;
}

bool Zone::LoadZones()
{
    World* world = World::GetPtr();
    DIR* dir = opendir(AREA_DIR);

    if (!dir)
    {
        return false;
    }

    dirent* cdir;
    while ((cdir = readdir(dir)))
    {
        if (cdir->d_name[0] == '.')
        {
            continue;
        }

        std::string filename = cdir->d_name;
        if (filename.find(".json") == std::string::npos)
        {
            continue;
        }

        Zone* zone = new Zone();
        std::string path = std::string(AREA_DIR) + filename;
        if (JsonFileSerializer::LoadFromFile(path, *zone, "Zone"))
        {
            world->AddZone(zone);
        }
        else
        {
            delete zone;
        }
    }

    closedir(dir);
    return true;
}

CEVENT(Zone, Autosave)
{
    zone_saves++;
    if (zone_saves >= 100)
    {
        Zone::SaveZones();
        zone_saves = 0;
    }
}
CEVENT(Zone, Shutdown)
{
    Zone::SaveZones();
}
