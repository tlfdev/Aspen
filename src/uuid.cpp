#include "uuid.h"

#include "serializationHelpers.h"

#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

Uuid::Uuid()
{
    _id = 0;
}
Uuid::Uuid(const Uuid& u)
{
    _id = u._id;
}
Uuid::~Uuid() {}
void Uuid::Initialize()
{
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    _id = generator();
    _id <<= 32;
    _id |= (0xFFFF & time(nullptr));
}
std::string Uuid::ToString() const
{
    std::stringstream st;
    st << std::setw(16) << std::setbase(16);
    st << _id;
    return st.str();
}
unsigned long long int Uuid::GetValue() const
{
    return _id;
}

// JSON serialization
void Uuid::ToJson(Json::Value& json) const
{
    json["id"] = (Json::Value::UInt64)_id;
}

void Uuid::FromJson(const Json::Value& json, int version)
{
    if (json.isMember("id") && json["id"].isUInt64())
    {
        _id = json["id"].asUInt64();
    }
    else
    {
        _id = 0;
    }
}

Uuid& Uuid::operator=(Uuid& u)
{
    _id = u._id;
    return *this;
}
bool Uuid::operator==(Uuid& u)
{
    return (u._id == _id);
}
