#pragma once

#include <string>

#include <json/json.h>

class Uuid
{
  protected:
    unsigned long long int _id;

  public:
    Uuid();
    Uuid(const Uuid& u);
    ~Uuid();
    void Initialize();
    std::string ToString() const;
    unsigned long long int GetValue() const;

    // JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);

    Uuid& operator=(Uuid& u);
    bool operator==(Uuid& u);
};
