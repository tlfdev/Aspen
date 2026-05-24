#include "exit.h"

#include "jsonSerializer.h"
#include "living.h"

#include <string>

Exit::Exit(VNUM to) : _to(to)
{
    _direction = nowhere;
}
Exit::Exit() : _to(EXIT_NOWHERE) {}

VNUM Exit::GetTo() const
{
    return _to;
}
void Exit::SetTo(VNUM to)
{
    _to = to;
}

ExitDirection Exit::GetDirection() const
{
    return _direction;
}
void Exit::SetDirection(ExitDirection dir)
{
    _direction = dir;
}

std::string Exit::GetName() const
{
    switch (_direction)
    {
        case north:
            return "north";
        case south:
            return "south";
        case east:
            return "east";
        case west:
            return "west";
        case northwest:
            return "northwest";
        case northeast:
            return "northeast";
        case southwest:
            return "southwest";
        case southeast:
            return "southeast";
        default:
            return "unknown";
    }
}

bool Exit::CanEnter(Living* mobile)
{
    return true;
}

void Exit::ToJson(Json::Value& json) const
{
    json["direction"] = static_cast<int>(_direction);
    json["to"] = _to;
}

void Exit::FromJson(const Json::Value& json, int version)
{
    _direction =
        static_cast<ExitDirection>(JsonSerializerHelpers::GetInt(json, "direction", static_cast<int>(nowhere)));
    _to = JsonSerializerHelpers::GetUInt(json, "to", EXIT_NOWHERE);
}
