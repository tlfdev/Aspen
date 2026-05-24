#include "staticObject.h"

#include "baseObject.h"
#include "conf.h"
#include "mud.h"
#include "objectManager.h"
#include "world.h"

#include <algorithm>
#include <functional>
#ifdef OLC
    #include "olc.h"
    #include "olcGroup.h"
    #include "olcManager.h"
#endif

StaticObject::StaticObject()
{
    _totalCount = 0;
}
StaticObject::~StaticObject() {}

std::string StaticObject::GetShort() const
{
    return _short;
}
void StaticObject::SetShort(const std::string& s)
{
    _short = s;
}
std::string StaticObject::GetPlural() const
{
    return _plural;
}
void StaticObject::SetPlural(const std::string& s)
{
    _plural = s;
}

unsigned int StaticObject::GetTotalCount() const
{
    return _totalCount;
}

unsigned int StaticObject::CountDescendants() const
{
    return (unsigned int)descendants.size();
}

bool StaticObject::IsDescendant(Entity* obj)
{
    std::vector<Entity*>::iterator it, itEnd;

    itEnd = descendants.end();
    for (it = descendants.begin(); it != itEnd; ++it)
    {
        if ((*it)->GetUuid() == obj->GetUuid())
        {
            return true;
        }
    }
    return false;
}
bool StaticObject::RemoveDescendant(Entity* obj)
{
    std::vector<Entity*>::iterator it, itEnd;

    itEnd = descendants.end();
    for (it = descendants.begin(); it != itEnd; ++it)
    {
        if ((*it)->GetUuid() == obj->GetUuid())
        {
            descendants.erase(it);
            return true;
        }
    }

    return false;
}

Entity* StaticObject::Create()
{
    Entity* obj = new Entity();
    obj->SetParent(this);
    obj->Initialize();
    Copy((BaseObject*)obj);
    _totalCount++;
    descendants.push_back(obj);
    return obj;
}
bool StaticObject::Recycle(Entity* obj)
{
    if (obj == nullptr)
    {
        return false;
    }

    if (IsDescendant(obj))
    {
        _totalCount--;
        return RemoveDescendant(obj);
    }

    return false;
}

bool StaticObject::RecycleContents()
{
    World* world = World::GetPtr();
    ObjectManager* omanager = world->GetObjectManager();
    std::vector<Entity*>::iterator it, itEnd;

    if (descendants.size())
    {
        itEnd = descendants.end();
        for (it = descendants.begin(); it != itEnd; ++it)
        {
            omanager->RecycleObject((*it));
        }
    }

    return true;
}

// JSON serialization
void StaticObject::ToJson(Json::Value& json) const
{
    using namespace JsonSerializerHelpers;

    // Serialize base object
    BaseObject::ToJson(json);

    // Serialize StaticObject-specific fields
    json["short"] = _short;
    json["plural"] = _plural;

    // Serialize component metas
    Json::Value componentsArray(Json::arrayValue);
    for (const auto* compMeta : _components)
    {
        if (compMeta)
        {
            Json::Value compJson;
            compJson["name"] = compMeta->GetName();
            // Component meta serialization would go here
            componentsArray.append(compJson);
        }
    }
    json["componentMetas"] = componentsArray;
}

void StaticObject::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    // Deserialize base object
    BaseObject::FromJson(json, version);

    // Deserialize StaticObject-specific fields
    _short = GetString(json, "short", "");
    _plural = GetString(json, "plural", "");

    // Deserialize component metas
    if (json.isMember("componentMetas") && json["componentMetas"].isArray())
    {
        World* world = World::GetPtr();
        const Json::Value& componentsArray = json["componentMetas"];

        for (const auto& compJson : componentsArray)
        {
            if (compJson.isMember("name"))
            {
                std::string cname = compJson["name"].asString();
                IComponentMeta* com = world->GetComponentFactory()->GetMeta(cname);
                if (com)
                {
                    _components.push_back(com);
                    // Component meta deserialization would go here
                }
            }
        }
    }
}

// initialize olcs.
bool InitializeStaticObjectOlcs()
{
    World* world = World::GetPtr();
    OlcManager* omanager = world->GetOlcManager();
    OlcGroup* sgroup = new OlcGroup();

    sgroup->SetInheritance(omanager->GetGroup(OLCGROUP::BaseObject));
    sgroup->AddEntry(new OlcStringEntry<StaticObject>(
        "short", "the title of the object seen in rooms", OF_NORMAL, OLCDT::STRING,
        std::bind(&StaticObject::GetShort, std::placeholders::_1),
        std::bind(&StaticObject::SetShort, std::placeholders::_1, std::placeholders::_2)));
    sgroup->AddEntry(new OlcStringEntry<StaticObject>(
        "plural", "Sets the plural of the object", OF_NORMAL, OLCDT::STRING,
        std::bind(&StaticObject::GetPlural, std::placeholders::_1),
        std::bind(&StaticObject::SetPlural, std::placeholders::_1, std::placeholders::_2)));
    omanager->AddGroup(OLCGROUP::STATIC, sgroup);
    return true;
}
