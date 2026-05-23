#include "property.h"

#include "conf.h"
#include "mud.h"
#include "serializationHelpers.h"
#include "variant.h"

#include <tinyxml2.h>

#include <string>
#include <vector>


Property::Property(const std::string& name, const Variant& value, Property* parent)
    : _name(name), _value(value), _parent(parent)
{
    _owner = nullptr;
}
Property::Property() : _name("root"), _parent(nullptr)
{
    _value = Variant();
    _owner = nullptr;
}
Property::~Property()
{
    RemoveSelf();
}

Variant Property::GetValue()
{
    return _value;
}
Variant& Property::GetPropertyRef(const std::string& name)
{
    Property* prop = FindProperty(name);
    if (!prop)
    {
        throw std::runtime_error("Property not found: Searched for " + name + ".");
    }
    else
    {
        return prop->_value;
    }
}

void Property::SetValue(const Variant& value)
{
    _value = value;
}
void Property::SetObject(BaseObject* obj)
{
    _owner = obj;
}

BaseObject* Property::GetOwner() const
{
    return _owner;
}

Property* Property::GetParent() const
{
    return _parent;
}
void Property::SetParent(Property* parent)
{
    _parent = parent;
}

void Property::AddProperty(Property* prop)
{
    prop->SetParent(this);
    _children.push_back(prop);
}
Property* Property::AddProperty(const std::string& name, const Variant& value)
{
    size_t dotInd = name.find_first_of(".");
    Property* prop = nullptr;

    if (dotInd != std::string::npos)
    {
        std::string childPart = name.substr(0, dotInd);
        std::string newName = name.substr(dotInd + 1);

        // first we need to make sure that childPart exists.
        prop = FindProperty(childPart);
        if (!prop)
        {
            prop = new Property(childPart, Variant(), this);
            _children.push_back(prop);
            return prop->AddProperty(newName, value);
        }
    }
    else
    {
        // we're at the last dot, we can create the node
        prop = new Property(name, value, this);
        _children.push_back(prop);
        return prop;
    }

    return prop;
}

void Property::RemoveSelf()
{
    if (!_parent)
        return;

    _parent->RemoveProperty(this);
    _parent = nullptr;
}
void Property::RemoveProperty(const std::string& name)
{
    Property* prop = FindProperty(name);
    if (!prop)
    {
        return;
    }

    for (auto it : prop->_children)
    {
        RemoveProperty(it);
    }
}
void Property::RemoveProperty(Property* prop)
{
    if (!prop)
    {
        return;
    }

    // first we have to remove all of its children first
    for (auto it : prop->_children)
    {
        Property* tmp = it;
        prop->RemoveProperty(tmp);
        delete tmp;
    }
    delete prop;
}

Property* Property::FindProperty(const std::string& name)
{
    std::vector<Property*>::iterator it, itEnd;
    // first check cache
    /*
      if (name == _lastSearchName)
        return _lastSearchRes;
    */

    size_t dotInd = name.find_first_of(".");
    if (dotInd != std::string::npos)
    {
        std::string childName = name.substr(0, dotInd);
        std::string newName = name.substr(dotInd + 1);

        itEnd = _children.end();
        for (it = _children.begin(); it != itEnd; ++it)
        {
            if ((*it)->_name == childName)
            {
                return ((*it)->FindProperty(newName));
            }
        }
    }

    itEnd = _children.end();
    for (it = _children.begin(); it != itEnd; ++it)
    {
        if ((*it)->_name == name)
        {
            return *it;
        }
    }

    return nullptr;
}

// JSON serialization
void Property::ToJson(Json::Value& json) const
{
    json["name"] = _name;

    // Serialize value
    Json::Value valueJson;
    _value.ToJson(valueJson);
    json["value"] = valueJson;

    // Serialize children
    if (!_children.empty())
    {
        Json::Value childrenArray(Json::arrayValue);
        for (const auto* child : _children)
        {
            if (child)
            {
                Json::Value childJson;
                child->ToJson(childJson);
                childrenArray.append(childJson);
            }
        }
        json["children"] = childrenArray;
    }
}

void Property::FromJson(const Json::Value& json, int version)
{
    if (json.isMember("name"))
    {
        _name = json["name"].asString();
    }

    // Deserialize value
    if (json.isMember("value"))
    {
        _value.FromJson(json["value"], version);
    }

    // Deserialize children
    if (json.isMember("children") && json["children"].isArray())
    {
        const Json::Value& childrenArray = json["children"];
        for (const auto& childJson : childrenArray)
        {
            Property* child = new Property();
            child->FromJson(childJson, version);
            child->SetParent(this);
            _children.push_back(child);
        }
    }
}
