/**
 * Modern JSON-based serialization framework
 * Replaces the old XML serialization with a cleaner, versioned approach
 */
#pragma once
#ifndef JSON_SERIALIZER_H
    #define JSON_SERIALIZER_H

    #include "conf.h"
    #include "mud.h"

    #include <fstream>
    #include <list>
    #include <memory>
    #include <stdexcept>
    #include <string>
    #include <vector>

    #include <json/json.h>

// Forward declarations
class World;

/**
 * Serialization exception for better error handling
 */
class SerializationException : public std::runtime_error
{
  public:
    explicit SerializationException(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * New serialization interface with versioning support
 */
class IJsonSerializable
{
  public:
    virtual ~IJsonSerializable() = default;

    /**
     * Serialize object to JSON
     * @param json The JSON value to write to
     */
    virtual void ToJson(Json::Value& json) const = 0;

    /**
     * Deserialize object from JSON
     * @param json The JSON value to read from
     * @param version The version of the saved data
     */
    virtual void FromJson(const Json::Value& json, int version) = 0;

    /**
     * Get the current version of this object's serialization format
     * Override this when you change the serialization format
     */
    virtual int GetSerializationVersion() const
    {
        return 1;
    }
};

/**
 * Helper class for reading/writing JSON files with versioning
 */
class JsonFileSerializer
{
  public:
    /**
     * Save an object to a JSON file with versioning
     */
    template <typename T>
    static bool SaveToFile(const std::string& filepath, const T& object, const std::string& typeName)
    {
        try
        {
            Json::Value root;
            root["version"] = object.GetSerializationVersion();
            root["type"] = typeName;

            Json::Value data;
            object.ToJson(data);
            root["data"] = data;

            std::ofstream file(filepath);
            if (!file.is_open())
            {
                return false;
            }

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  "; // Pretty print
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(root, &file);
            file << std::endl;

            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    /**
     * Load an object from a JSON file with version checking
     */
    template <typename T>
    static bool LoadFromFile(const std::string& filepath, T& object, const std::string& expectedType)
    {
        try
        {
            std::ifstream file(filepath);
            if (!file.is_open())
            {
                return false;
            }

            Json::Value root;
            Json::CharReaderBuilder builder;
            std::string errs;

            if (!Json::parseFromStream(builder, file, &root, &errs))
            {
                throw SerializationException("JSON parse error: " + errs);
            }

            // Check type
            if (root.isMember("type") && root["type"].asString() != expectedType)
            {
                throw SerializationException("Type mismatch: expected " + expectedType + ", got " +
                                             root["type"].asString());
            }

            // Get version
            int version = root.isMember("version") ? root["version"].asInt() : 1;

            // Deserialize
            if (root.isMember("data"))
            {
                object.FromJson(root["data"], version);
            }
            else
            {
                // Legacy support - root is the data
                object.FromJson(root, version);
            }

            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }
};

/**
 * Helper functions for common JSON operations
 */
namespace JsonSerializerHelpers
{

/**
 * Serialize a vector of serializable objects
 */
template <typename T>
void SerializeVector(Json::Value& json, const std::string& key, const std::vector<T*>& items)
{
    Json::Value array(Json::arrayValue);
    for (const auto* item : items)
    {
        if (item)
        {
            Json::Value itemJson;
            item->ToJson(itemJson);
            array.append(itemJson);
        }
    }
    json[key] = array;
}

/**
 * Deserialize a vector of serializable objects
 */
template <typename T>
void DeserializeVector(const Json::Value& json, const std::string& key, std::vector<T*>& items, int version)
{
    if (!json.isMember(key) || !json[key].isArray())
    {
        return;
    }

    const Json::Value& array = json[key];
    for (const auto& itemJson : array)
    {
        auto* item = new T();
        item->FromJson(itemJson, version);
        items.push_back(item);
    }
}

/**
 * Serialize a list of serializable objects
 */
template <typename T>
void SerializeList(Json::Value& json, const std::string& key, const std::list<T*>& items)
{
    Json::Value array(Json::arrayValue);
    for (const auto* item : items)
    {
        if (item)
        {
            Json::Value itemJson;
            item->ToJson(itemJson);
            array.append(itemJson);
        }
    }
    json[key] = array;
}

/**
 * Deserialize a list of serializable objects
 */
template <typename T>
void DeserializeList(const Json::Value& json, const std::string& key, std::list<T*>& items, int version)
{
    if (!json.isMember(key) || !json[key].isArray())
    {
        return;
    }

    const Json::Value& array = json[key];
    for (const auto& itemJson : array)
    {
        auto* item = new T();
        item->FromJson(itemJson, version);
        items.push_back(item);
    }
}

/**
 * Serialize a vector of strings
 */
inline void SerializeStringVector(Json::Value& json, const std::string& key, const std::vector<std::string>& items)
{
    Json::Value array(Json::arrayValue);
    for (const auto& item : items)
    {
        array.append(item);
    }
    json[key] = array;
}

/**
 * Deserialize a vector of strings
 */
inline void DeserializeStringVector(const Json::Value& json, const std::string& key, std::vector<std::string>& items)
{
    if (!json.isMember(key) || !json[key].isArray())
    {
        return;
    }

    const Json::Value& array = json[key];
    for (const auto& item : array)
    {
        if (item.isString())
        {
            items.push_back(item.asString());
        }
    }
}

/**
 * Safe string getter with default
 */
inline std::string GetString(const Json::Value& json, const std::string& key, const std::string& defaultValue = "")
{
    if (json.isMember(key) && json[key].isString())
    {
        return json[key].asString();
    }
    return defaultValue;
}

/**
 * Safe int getter with default
 */
inline int GetInt(const Json::Value& json, const std::string& key, int defaultValue = 0)
{
    if (json.isMember(key) && json[key].isInt())
    {
        return json[key].asInt();
    }
    return defaultValue;
}

/**
 * Safe unsigned int getter with default
 */
inline unsigned int GetUInt(const Json::Value& json, const std::string& key, unsigned int defaultValue = 0)
{
    if (json.isMember(key) && json[key].isUInt())
    {
        return json[key].asUInt();
    }
    return defaultValue;
}

/**
 * Safe bool getter with default
 */
inline bool GetBool(const Json::Value& json, const std::string& key, bool defaultValue = false)
{
    if (json.isMember(key) && json[key].isBool())
    {
        return json[key].asBool();
    }
    return defaultValue;
}

/**
 * Safe 64-bit int getter with default
 */
inline unsigned long long GetUInt64(const Json::Value& json, const std::string& key,
                                    unsigned long long defaultValue = 0)
{
    if (json.isMember(key) && json[key].isUInt64())
    {
        return json[key].asUInt64();
    }
    return defaultValue;
}
} // namespace JsonSerializerHelpers

#endif // JSON_SERIALIZER_H
