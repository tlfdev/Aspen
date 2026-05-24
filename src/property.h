/*A Property tree, used for attaching and storing properties on objects.
 *Wrritten by CPPMan
 */
#pragma once
#ifndef _PROPERTY_HPP
    #define _PROPERTY_HPP
    #include "conf.h"
    #include "mud.h"
    #include "variant.h"

    #include <string>
    #include <vector>

    #include <json/json.h>

class Property
{
  protected:
    std::string _name;
    Variant _value;
    Property* _parent;
    std::vector<Property*> _children;
    BaseObject* _owner;

  public:
    Property(const std::string& name, const Variant& value, Property* parent = nullptr);
    Property();
    ~Property();
    Variant GetValue();
    Variant& GetPropertyRef(const std::string& name);
    void SetValue(const Variant& value);
    void SetObject(BaseObject* obj);
    BaseObject* GetOwner() const;
    Property* GetParent() const;
    void SetParent(Property* parent);
    // Adds a child to this Property.
    void AddProperty(Property* prop);
    // Adds a Property to this Property.
    Property* AddProperty(const std::string& name, const Variant& value);

    // Removes THIS Property from its own parent.
    void RemoveSelf();

    // Removes a Property from this Property (and all of its children)
    void RemoveProperty(const std::string& name);
    // Removes a Property from this Property (and all of its children)
    void RemoveProperty(Property* prop);
    // Finds a Property based on its name.
    Property* FindProperty(const std::string& name);

    // JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);
};
#endif /* _Property_HPP */
