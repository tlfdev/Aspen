#pragma once
#include "baseObject.h"
#include "conf.h"
#include "mud.h"

#include <list>

/**
The object container holds all in-game objects and manages adding, removing and searching through that list.

This extra class is necessary because rooms initially had a lot of overhead inheriting from entity.
We need a basic class that inherits BaseObject and contains everything, but which also does not contain StaticObject
data.
*/
class Entity;
class ObjectContainer : public BaseObject
{
  protected:
    std::list<Entity*> _contents;

  public:
    ObjectContainer() = default;
    ~ObjectContainer() = default;
    /*
     *Returns a pointer to a list of objects that the object contains.
     *Return: A pointer to an std::list of pointers to objects.
     */
    std::list<Entity*>* GetContents();
    virtual bool CanReceive(Entity* obj) const;
    /*
     *Called when an object is moved into another object.
     *For example, if you put a sword in a container, the ObjectEnter would be called on the container with the sword
     *being passed as an argument. Param: [in] the object entering the current object.
     */
    virtual void ObjectEnter(Entity* obj);
    virtual void ObjectLeave(Entity* obj);

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};
