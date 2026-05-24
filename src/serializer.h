/*
 *This class defines the basic interface for an object that can be serialized.
 *Now uses JSON serialization exclusively.
 */
#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "conf.h"
#include "jsonSerializer.h"
#include "mud.h"

// ISerializable now just inherits from IJsonSerializable
// Kept for backward compatibility with existing code
class ISerializable : public IJsonSerializable
{
  public:
    virtual ~ISerializable() {}
};
#endif
