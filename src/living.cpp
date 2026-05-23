#include "living.h"

#include "delayedEvent.h"
#include "event.h"
#include "mud.h"
#include "world.h"

#include <tinyxml2.h>

Living::Living()
{
    events.RegisterEvent("HeartBeat");

    _position = POSITION_STANDING;
    _gender = Gender::Neuter;
    _following = nullptr;
}

void Living::EnterGame() {}
void Living::LeaveGame() {}

void Living::Update()
{
    events.CallEvent("HeartBeat", nullptr, (void*)this);
}

bool Living::IsLiving() const
{
    return true;
}

Gender Living::GetGender() const
{
    return _gender;
}
void Living::SetGender(Gender gender)
{
    _gender = gender;
}

unsigned int Living::GetPosition() const
{
    return _position;
}
void Living::SetPosition(unsigned int pos)
{
    _position = pos;
}

bool Living::AddAttribute(Attribute* attr)
{
    _attributes.push_back(attr);
    return true;
}
void Living::FindAttribute(int apply, int id, std::vector<Attribute*>& results)
{
    for (Attribute* attr : _attributes)
    {
        if (attr->GetApply() == apply && attr->GetId() == id)
        {
            results.push_back(attr);
        }
    }
}
void Living::FindAttribute(int type, std::vector<Attribute*>& results)
{
    for (Attribute* attr : _attributes)
    {
        if (attr->GetType() == type)
        {
            results.push_back(attr);
        }
    }
}

// JSON serialization
void Living::ToJson(Json::Value& json) const
{
    // Serialize base entity
    Entity::ToJson(json);

    // Serialize Living-specific fields
    json["gender"] = static_cast<int>(_gender);
    json["position"] = _position;
}

void Living::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    // Deserialize base entity
    Entity::FromJson(json, version);

    // Deserialize Living-specific fields
    _gender = static_cast<Gender>(GetInt(json, "gender", static_cast<int>(Gender::Neuter)));
    _position = GetUInt(json, "position", POSITION_STANDING);
}
