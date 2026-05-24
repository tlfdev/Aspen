/**
 * Contains: BaseObject
 *
 * The BaseObject class is responsible for holding all information needed for game objects.
 * This includes rooms and entities.
 */
#pragma once
#ifndef BASE_OBJECT_H
    #define BASE_OBJECT_H
    #include "command.h"
    #include "conf.h"
    #include "event.h"
    #include "eventManager.h"
    #include "eventargs.h"
    #include "mud.h"
    #include "property.h"
    #include "serializer.h"
    #include "variant.h"

    #include <list>
    #include <string>

/**
 * Holds information relevant to all objects.
 */
class BaseObject : public IJsonSerializable
{
  protected:
    std::vector<Component*> _components;
    VNUM _onum;
    std::string _name;
    std::string _desc;
    std::string _script; // the scripting associated with this object.
    Zone* _zone;

  public:
    EventManager events;

    BaseObject();
    virtual ~BaseObject();

    /**
     * Gets the name of the object.
     */
    virtual std::string GetName() const;
    /**
     * Sets the name of the object.
     * @param [in] name The name of the object.
     */
    virtual void SetName(const std::string& s);
    /**
     * Gets the description of the object.
     *
     * The description is what will be seen when a player or NPC looks at the object.
     * @return The description of the object.
     */
    virtual std::string GetDescription() const;
    /**
     * Sets the object's description.
     * @param [in] description The new description for the object.
     */
    virtual void SetDescription(const std::string& s);
    /**
     * Gets the scripting associated with this object.
     *
     * The script for the object is the angelscript code that runs when objects are initialized.
     * @return The full code for the script associated with this object.
     */
    virtual std::string GetScript() const;
    /**
     * Sets the script code associated with this object.
     * @param [in] script The new code.
     */
    virtual void SetScript(const std::string& script);
    /**
     * Gets a pointer to the zone object.
     *
     * The zone object is the object which owns this.
     * In otherwords, this is the zone that this object belongs to.
     */
    Zone* GetZone() const;
    /**
     * Sets the zone object that this object belongs to.
     * @param [in] s The new zone object.
     */
    void SetZone(Zone* s);
    /**
     * Return The object's vnum.
     *
     * The VNum of an object is the identifier used to identify the object.
     */
    VNUM GetOnum() const;
    /**
     * Sets the object's num.
     * @param [in] num the vnum for the object.
     */
    void SetOnum(VNUM num);
    /**
     * Adds the specified component to the list.
     *
     * @param [in] component a pointer to the component object.
     * @return true if the component could be added, false otherwise.
     */
    bool AddComponent(Component* component);
    /**
     * Removes the specified component from the list.
     *
     * @param [in] component a pointer to the component to be added.
     * @Return true on success, false on failure.
     */
    bool RemoveComponent(Component* component);
    /**
     * Checks for the existance of a component by name.
     *
     * @param [in] name the name of the component.
     * @return True if the component exists, false otherwise.
     */
    bool HasComponent(const std::string& name);
    /**
     * Retrieves a pointer to the specified component.
     *
     * @param [in] name the name of the component.
     * @return A pointer to the component, or NULL if it was not found.
     */
    Component* GetComponent(const std::string& name);
    /**
     * Attaches each component to the object it belongs to.
     *
     * When an component is attached to an object, it looks for all dependencies.
     * This can also be used to process other component-specific logic.
     */
    void Attach();
    // TODO: finish docs when this code is cleaned.
    virtual std::string Identify(Player* mobile);

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }

    virtual void Copy(BaseObject* obj) const;
    virtual std::string DoLook(Player* mobile);
    virtual bool IsPlayer() const;
    virtual bool IsLiving() const;
    virtual bool IsRoom() const;
    virtual bool IsNpc() const;
    virtual bool IsObject() const;
};

bool InitializeBaseObjectOlcs();
#endif
