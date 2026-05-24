#include "baseObject.h"

#include "component.h"
#include "editor.h"
#include "entity.h"
#include "eventargs.h"
#include "olc.h"
#include "olcManager.h"
#include "serializationHelpers.h"
#include "utils.h"
#include "world.h"

#include <list>
#include <sstream>
#include <string>

BaseObject::BaseObject()
{
    events.RegisterEvent("PreLook");
    events.RegisterEvent("PostLook");
    events.RegisterEvent("OnEnter");
    events.RegisterEvent("OnExit");
    events.RegisterEvent("OnLook");

    _name = "A blank object";
    _desc = "You see nothing special.";
    _onum = 0;
    _zone = nullptr;
}
BaseObject::~BaseObject()
{
    for (auto it : _components)
    {
        delete it;
    }
}

std::string BaseObject::GetName() const
{
    return _name;
}
void BaseObject::SetName(const std::string& s)
{
    _name = s;
}

std::string BaseObject::GetDescription() const
{
    return _desc;
}
void BaseObject::SetDescription(const std::string& s)
{
    _desc = s;
}

std::string BaseObject::GetScript() const
{
    return _script;
}
void BaseObject::SetScript(const std::string& script)
{
    _script = script;
}

Zone* BaseObject::GetZone() const
{
    return _zone;
}
void BaseObject::SetZone(Zone* s)
{
    _zone = s;
}

VNUM BaseObject::GetOnum() const
{
    return _onum;
}
void BaseObject::SetOnum(VNUM num)
{
    _onum = num;
}

bool BaseObject::AddComponent(Component* component)
{
    if (component == nullptr)
    {
        return false;
    }
    if (HasComponent(component->GetMeta()->GetName()))
    {
        return false;
    }

    _components.push_back(component);
    component->Attach(this);
    return true;
}
bool BaseObject::RemoveComponent(Component* component)
{
    std::vector<Component*>::iterator it, itEnd;

    itEnd = _components.end();
    for (it = _components.begin(); it != itEnd; ++it)
    {
        if ((*it) == component)
        {
            component->Detach();
            _components.erase(it);
            delete (*it);
            return true;
        }
    }

    return false;
}
bool BaseObject::HasComponent(const std::string& name)
{
    for (auto it : _components)
    {
        if (it->GetMeta()->GetName() == name)
        {
            return true;
        }
    }

    return false;
}
Component* BaseObject::GetComponent(const std::string& name)
{
    for (auto it : _components)
    {
        if (it->GetMeta()->GetName() == name)
        {
            return it;
        }
    }

    return nullptr;
}

void BaseObject::Attach()
{
    for (auto it : _components)
    {
        it->Attach(this);
    }
}

// JSON serialization implementation
void BaseObject::ToJson(Json::Value& json) const
{
    json["name"] = _name;
    json["desc"] = _desc;
    json["script"] = _script;
    json["onum"] = _onum;

    // Serialize components
    Json::Value componentsArray(Json::arrayValue);
    for (const auto* comp : _components)
    {
        if (comp)
        {
            Json::Value compJson;
            compJson["type"] = comp->GetMeta()->GetName();
            comp->ToJson(compJson);
            componentsArray.append(compJson);
        }
    }
    json["components"] = componentsArray;
}

void BaseObject::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    _name = GetString(json, "name", "A blank object");
    _desc = GetString(json, "desc", "You see nothing special.");
    _script = GetString(json, "script", "");
    _onum = GetUInt(json, "onum", 0);

    // Deserialize components
    if (json.isMember("components") && json["components"].isArray())
    {
        World* world = World::GetPtr();
        const Json::Value& componentsArray = json["components"];

        for (const auto& compJson : componentsArray)
        {
            if (compJson.isMember("type"))
            {
                std::string compType = compJson["type"].asString();
                Component* comp = world->CreateComponent(compType);
                if (comp)
                {
                    comp->FromJson(compJson, version);
                    AddComponent(comp);
                }
            }
        }
    }
}

void BaseObject::Copy(BaseObject* obj) const
{
    /**
     * @todo move this to entity.
     */
    /*
        for (auto it: _aliases)
            {
                obj->AddAlias(it);
            }
    */

    obj->SetName(_name);
    obj->SetOnum(_onum);
    obj->SetDescription(_desc);
    obj->SetScript(_script);
}

std::string BaseObject::Identify(Player* mobile)
{
    std::stringstream st;
    st << GetName() << std::endl;
    st << "Originating zone: " << GetZone()->GetName() << std::endl;
    return st.str();
}

std::string BaseObject::DoLook(Player* mobile)
{
    std::string str;

    LookArgs* args = new LookArgs(mobile, this, str);
    events.CallEvent("PreLook", args, (void*)mobile);
    str += Capitalize(this->GetName()) + "\n";
    str += this->GetDescription() + "\n";
    events.CallEvent("PostLook", args, (void*)mobile);
    delete args;
    return str;
}

bool BaseObject::IsNpc() const
{
    return false;
}
bool BaseObject::IsPlayer() const
{
    return false;
}
bool BaseObject::IsLiving() const
{
    if (IsPlayer() || IsNpc())
    {
        return true;
    }

    return false;
}
bool BaseObject::IsRoom() const
{
    return false;
}
bool BaseObject::IsObject() const
{
    return false;
}

bool InitializeBaseObjectOlcs()
{
    World* world = World::GetPtr();
    OlcManager* omanager = world->GetOlcManager();
    OlcGroup* group = new OlcGroup();

    group->AddEntry(
        new OlcStringEntry<BaseObject>("name", "the name of the object", OF_NORMAL, OLCDT::STRING,
                                       std::bind(&BaseObject::GetName, std::placeholders::_1),
                                       std::bind(&BaseObject::SetName, std::placeholders::_1, std::placeholders::_2)));
    group->AddEntry(new OlcEditorEntry<BaseObject>(
        "description", "the description of the object", OF_NORMAL, OLCDT::EDITOR,
        std::bind(&BaseObject::GetDescription, std::placeholders::_1),
        std::bind(&BaseObject::SetDescription, std::placeholders::_1, std::placeholders::_2)));
    omanager->AddGroup(OLCGROUP::BaseObject, group);
    return true;
}
