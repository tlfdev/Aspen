/*
 *This class defines the basic interface for an object that can be serialized.
 *NOTE: This is the LEGACY XML interface. New code should use IJsonSerializable from jsonSerializer.h
 *This is kept for backward compatibility during transition.
 */
#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "conf.h"
#include "jsonSerializer.h"
#include "mud.h"

#include <tinyxml2.h>

// LEGACY: Old XML-based serialization interface
// Deprecated: Use IJsonSerializable instead
class ISerializable
{
  public:
    virtual ~ISerializable() {}
    virtual void Serialize(tinyxml2::XMLElement* root) = 0;
    virtual void Deserialize(tinyxml2::XMLElement* root) = 0;
};
#endif
