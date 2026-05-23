/**
 * Contains: Entity
 *
 * Entities are generally what all in-game objects inherit from.
 */
#pragma once
#ifndef ENTITY_H
    #define ENTITY_H
    #include "baseObject.h"
    #include "command.h"
    #include "event.h"
    #include "eventManager.h"
    #include "eventargs.h"
    #include "mud.h"
    #include "objectContainer.h"
    #include "property.h"
    #include "uuid.h"
    #include "variant.h"

    #include <tinyxml2.h>

    #include <list>
    #include <map>
    #include <string>

class Entity : public ObjectContainer
{
  protected:
    ObjectContainer* _location;
    StaticObject* _parent;
    Uuid _uuid;
    std::vector<std::string> _aliases;

    std::string _short;

  public:
    Entity();
    virtual ~Entity() = default;

    /**
     * Returns the short description of the entity.
     *
     * The short description is what players will see when the entity is not in their inventory.
     * For example, a sword's short description might be:
     * A long sword lies here.
     */
    virtual std::string GetShort() const;
    /**
     * Sets the entities short description.
     *
     * @param [in] s the new short description.
     */
    virtual void SetShort(const std::string& s);

    /**
     * Returns the location of this object.
     *
     * @return the entities location, or nullptr if the object does not have  a parent location.
     */
    ObjectContainer* GetLocation() const;
    /**
     * Sets the location of the object
     *
     * @param [in] location The object to move the entity to.
     */
    void SetLocation(ObjectContainer* location);

    /**
     * Gets the static object associated with this entity.
     */
    StaticObject* GetParent() const;
    /**
     * Sets the static object associated with this object.
     *
     * @param [in] parent the new parent for this object.
     */
    void SetParent(StaticObject* parent);

    /**
     * Attempts to moves this object to the target
     *
     * @param [in] targ the destination where the object needs to go.
     *Return: true on success, false if the object can not move.
     */
    virtual bool MoveTo(ObjectContainer* targ);
    /**
     * Removes this object from the room it is in, if possible.
     */
    bool FromRoom();

    /**
     * Initializes the entity after loaded.
     */
    virtual void Initialize();
    /**
     * Gets the UUID associated with the entity.
     */
    Uuid& GetUuid();
    /**
     * Adds the specified alias to the object.
     *
     * Aliases are other identifiers by which an object can be known.
     * For example, a sword could be known as a sword or by a name.
     * @param [in] alias the name of the alias to add.
     * @return True if the alias could be added, false otherwise.
     */
    bool AddAlias(const std::string& alias);
    /**
     * checks to see if the alias exists.
     *
     * @param: [in] the name of the alias.
     * @return True on success, false on failure.
     */
    bool AliasExists(const std::string& name);
    /**
     * Returns a pointer to the list of aliases.
     * @return a pointer to a vector of strings.
     */
    std::vector<std::string>* GetAliases();
    /**
     * Returns stats for this object.
     *
     * @param [in] mob the player requesting identification.
     */
    virtual std::string Identify(Player* mob);
    virtual bool IsObject() const;

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};

/**
 * Initializes OLCs attached to this entity.
 */
bool InitializeEntityOlcs();
#endif
