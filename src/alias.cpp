#include "alias.h"

#include "conf.h"
#include "jsonSerializer.h"
#include "mud.h"
#include "player.h"
#include "world.h"

#include <string>
#include <vector>

Alias::Alias(const std::string& name) : _name(name) {}

std::string Alias::GetName() const
{
    return _name;
}
void Alias::SetName(const std::string& name)
{
    _name = name;
}

void Alias::ProcessCommands(Player* mobile)
{
    World* world = World::GetPtr();

    for (auto it : _aliases)
    {
        world->DoCommand(mobile, it);
    }
}

void Alias::AddCommand(const std::string& command)
{
    if (!command.empty())
    {
        _aliases.push_back(command);
    }
}
void Alias::ClearCommands()
{
    _aliases.clear();
}
void Alias::ListCommands(std::vector<std::string>& commands)
{
    for (auto it : _aliases)
    {
        commands.push_back(it);
    }
}

void Alias::ToJson(Json::Value& json) const
{
    json["name"] = _name;
    JsonSerializerHelpers::SerializeStringVector(json, "aliases", _aliases);
}

void Alias::FromJson(const Json::Value& json, int version)
{
    _name = JsonSerializerHelpers::GetString(json, "name");
    _aliases.clear();
    JsonSerializerHelpers::DeserializeStringVector(json, "aliases", _aliases);
}
