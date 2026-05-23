#include "entity.h"

#include "command.h"
#include "editor.h"
#include "event.h"
#include "eventManager.h"
#include "eventargs.h"
#include "objectManager.h"
#include "olc.h"
#include "room.h"
#include "utils.h"
#include "world.h"

#include <tinyxml2.h>

#include <list>
#include <map>
#include <sstream>
#include <string>
#ifdef MODULE_SCRIPTING
    #include "scripts/script.h"
#endif

Entity::Entity() : _location(nullptr), _parent(nullptr)
{
    events.RegisterEvent("PostLook");
    events.RegisterEvent("PreLook");
}

std::string Entity::GetShort() const
{
    return _short;
}
void Entity::SetShort(const std::string& s)
{
    _short = s;
}

ObjectContainer* Entity::GetLocation() const
{
    return _location;
}
void Entity::SetLocation(ObjectContainer* location)
{
    _location = location;
}

StaticObject* Entity::GetParent() const
{
    return _parent;
}
void Entity::SetParent(StaticObject* parent)
{
    _parent = parent;
}

bool Entity::MoveTo(ObjectContainer* targ)
{
    if (targ->CanReceive(this))
    {
        if (_location)
        {
            _location->ObjectLeave(this);
        }
        _location = targ;
        targ->ObjectEnter(this);
        return true;
    }

    return false;
}

bool Entity::FromRoom()
{
    Room* loc = (Room*)_location;
    if (!_location || !_location->IsRoom())
    {
        return false;
    }
    loc->ObjectLeave(this);
    loc->events.CallEvent("OnExit", nullptr, (void*)this);

    return true;
}

void Entity::Initialize()
{
    _uuid.Initialize();
}
Uuid& Entity::GetUuid()
{
    return _uuid;
}
bool Entity::AddAlias(const std::string& alias)
{
    if (AliasExists(alias) && !alias.empty())
    {
        return false;
    }

    _aliases.push_back(alias);
    return true;
}
bool Entity::AliasExists(const std::string& name)
{
    for (auto it : _aliases)
    {
        if (it == name)
        {
            return true;
        }
    }

    return false;
}
std::vector<std::string>* Entity::GetAliases()
{
    return &_aliases;
}

std::string Entity::Identify(Player* mob)
{
    std::stringstream st;
    st << BaseObject::Identify(mob);
    st << "UUID: " << _uuid.ToString() << std::endl;
    return st.str();
}

bool Entity::IsObject() const
{
    return true;
}

bool InitializeEntityOlcs()
{
    World* world = World::GetPtr();
    OlcManager* omanager = world->GetOlcManager();
    OlcGroup* group = new OlcGroup();
    group->SetInheritance(omanager->GetGroup(OLCGROUP::BaseObject));
    group->AddEntry(
        new OlcStringEntry<Entity>("short", "the title of the object seen in rooms", OF_NORMAL, OLCDT::STRING,
                                   std::bind(&Entity::GetShort, std::placeholders::_1),
                                   std::bind(&Entity::SetShort, std::placeholders::_1, std::placeholders::_2)));

    omanager->AddGroup(OLCGROUP::Entity, group);
    return true;
}

// JSON serialization
void Entity::ToJson(Json::Value& json) const
{
    using namespace JsonSerializerHelpers;

    // Serialize base object
    BaseObject::ToJson(json);

    // Serialize ObjectContainer
    ObjectContainer::ToJson(json);

    // Serialize Entity-specific fields
    json["short"] = _short;
    json["location"] = (_location ? _location->GetOnum() : 0);

    // Serialize UUID
    Json::Value uuidJson;
    _uuid.ToJson(uuidJson);
    json["uuid"] = uuidJson;

    // Serialize aliases
    SerializeStringVector(json, "aliases", _aliases);
}

void Entity::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    // Deserialize base object
    BaseObject::FromJson(json, version);

    // Deserialize ObjectContainer
    ObjectContainer::FromJson(json, version);

    // Deserialize Entity-specific fields
    _short = GetString(json, "short", "");

    int loc = GetInt(json, "location", 0);
    if (!loc)
    {
        _location = nullptr;
    }
    else
    {
        World* world = World::GetPtr();
        ObjectManager* omanager = world->GetObjectManager();
        _location = omanager->GetRoom(loc);
    }

    // Deserialize UUID
    if (json.isMember("uuid"))
    {
        _uuid.FromJson(json["uuid"], version);
    }

    // Deserialize aliases
    DeserializeStringVector(json, "aliases", _aliases);

    // Notify that object was loaded
    World* world = World::GetPtr();
    world->events.CallEvent("ObjectLoaded", nullptr, this);
}
