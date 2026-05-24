#pragma once
#ifndef STATIC_OBJECT_H
    #define STATIC_OBJECT_H
    #include "baseObject.h"
    #include "componentMeta.hpp"
    #include "conf.h"
    #include "mud.h"
    #include "olc.h"


class Npc;
class StaticObject : public BaseObject
{
    std::string _plural;
    std::string _short; // the description you see in a room.
    unsigned int _totalCount;
    std::vector<Entity*> descendants;
    std::vector<IComponentMeta*> _components;

  public:
    StaticObject();
    ~StaticObject();
    virtual std::string GetShort() const;
    virtual void SetShort(const std::string& s);
    virtual std::string GetPlural() const;
    virtual void SetPlural(const std::string& s);
    unsigned int GetTotalCount() const;
    unsigned int CountDescendants() const;
    bool IsDescendant(Entity* obj);
    bool RemoveDescendant(Entity* obj);
    Entity* Create();
    bool Recycle(Entity* obj);
    bool RecycleContents();

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};

bool InitializeStaticObjectOlcs();
#endif
