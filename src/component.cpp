#include "component.h"

#include "eventargs.h"
#include "exception.h"
#include "world.h"

#include <vector>

void Component::Initialize()
{
    _attached = false;

    events.RegisterEvent("OnCreate");
    events.RegisterEvent("OnDestroy");
    events.RegisterEvent("OnAttach");
    events.RegisterEvent("OnDetach");
    events.CallEvent("OnCreate", nullptr, (void*)this);
}

Component::Component(IComponentMeta* parent) : _parent(parent) {}
Component::Component()
{
    _parent = nullptr;
}
Component::~Component()
{
    events.CallEvent("OnDestroy", nullptr, (void*)this);
}

// JSON serialization
void Component::ToJson(Json::Value& json) const
{
    if (_parent)
    {
        json["type"] = _parent->GetName();
    }
    // Serialize component variables/properties
    // Note: Property class would need JSON serialization support
    // For now, we'll leave this minimal
}

void Component::FromJson(const Json::Value& json, int version)
{
    // Deserialize component variables/properties
    // Note: Property class would need JSON serialization support
    // The component type is already handled by BaseObject
}

void Component::SetObject(BaseObject* obj)
{
    _object = obj;
}
BaseObject* Component::GetObject() const
{
    return _object;
}

IComponentMeta* Component::GetMeta() const
{
    return _parent;
}

void Component::Attach(BaseObject* obj)
{
    World* world = World::GetPtr();
    std::vector<std::string> dependencies;

    if (_attached)
    {
        return;
    }

    _parent->GetDependencies(&dependencies);
    for (auto it : dependencies)
    {
        obj->AddComponent(world->CreateComponent(it));
    }

    ComponentAttachedArgs arg(obj);
    events.CallEvent("OnAttach", &arg, (void*)this);
    _attached = true;
    SetObject(obj);
}
void Component::Detach()
{
    if (!_attached)
    {
        return;
    }

    SetObject(nullptr);
    _attached = false;
    events.CallEvent("OnDetach", nullptr, (void*)this);
}
