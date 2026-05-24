#include "boardPost.h"

#include <string>

#include "../../conf.h"
#include "../../mud.h"
#include "../../player.h"
#include "../../uuid.h"
#include "../modules.h"

#ifdef MODULE_BOARD

BoardPost::BoardPost() {}
BoardPost::BoardPost(const std::string& s, const std::string& m)
{
    _message = m;
    _subject = s;
}
BoardPost::~BoardPost() {}

std::string BoardPost::GetSubject() const
{
    return _subject;
}
void BoardPost::SetSubject(const std::string& s)
{
    _subject = s;
}
std::string BoardPost::GetMessage() const
{
    return _message;
}
void BoardPost::SetMessage(const std::string& m)
{
    _message = m;
}
std::string BoardPost::GetPoster() const
{
    return _poster;
}

bool BoardPost::IsPoster(Player* mobile)
{
    return (mobile->GetRealUuid() == _pid);
}
void BoardPost::SetPoster(Player* mobile)
{
    _poster = mobile->GetName();
    _pid = mobile->GetUuid();
}

// JSON serialization
void BoardPost::ToJson(Json::Value& json) const
{
    json["subject"] = _subject;
    json["message"] = _message;
    json["poster"] = _poster;

    // Serialize UUID
    Json::Value uuidJson;
    _pid.ToJson(uuidJson);
    json["pid"] = uuidJson;
}

void BoardPost::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    _subject = GetString(json, "subject", "");
    _message = GetString(json, "message", "");
    _poster = GetString(json, "poster", "");

    // Deserialize UUID
    if (json.isMember("pid"))
    {
        _pid.FromJson(json["pid"], version);
    }
}
#endif
