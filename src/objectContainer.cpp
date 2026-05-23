#include "objectContainer.h"

#include "baseObject.h"
#include "conf.h"
#include "entity.h"
#include "mud.h"
#include "serializationHelpers.h"

#include <tinyxml2.h>

#include <list>

std::list<Entity*>* ObjectContainer::GetContents()
{
    return &_contents;
}

bool ObjectContainer::CanReceive(Entity* obj) const
{
    return true;
}
void ObjectContainer::ObjectLeave(Entity* obj)
{
    std::list<Entity*>::iterator it, itEnd;

    itEnd = _contents.end();
    for (it = _contents.begin(); it != itEnd; ++it)
    {
        if ((*it) == obj)
        {
            it = _contents.erase(it);
            break;
        }
    }
}
void ObjectContainer::ObjectEnter(Entity* obj)
{
    _contents.push_back(obj);
}

// JSON serialization
void ObjectContainer::ToJson(Json::Value& json) const
{
    using namespace JsonSerializerHelpers;

    // Serialize base object
    BaseObject::ToJson(json);

    // Serialize contents
    SerializeList<Entity>(json, "contents", _contents);
}

void ObjectContainer::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    // Deserialize base object
    BaseObject::FromJson(json, version);

    // Deserialize contents
    DeserializeList<Entity>(json, "contents", _contents, version);

    // Set locations
    for (auto* item : _contents)
    {
        if (item)
        {
            item->SetLocation(this);
        }
    }
}
