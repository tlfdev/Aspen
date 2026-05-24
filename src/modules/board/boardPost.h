#pragma once
#include <string>

#include "../../conf.h"
#include "../../mud.h"
#include "../../player.h"
#include "../../uuid.h"
#include "../modules.h"
#include <json/json.h>

#ifdef MODULE_BOARD

class BoardPost
{
    std::string _subject;
    std::string _message;
    std::string _poster;
    Uuid _pid;

  public:
    BoardPost();
    BoardPost(const std::string& s, const std::string& m);
    ~BoardPost();
    std::string GetSubject() const;
    void SetSubject(const std::string& s);
    std::string GetMessage() const;
    void SetMessage(const std::string& m);
    std::string GetPoster() const;
    bool IsPoster(Player* mobile);
    void SetPoster(Player* mobile);

    // JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);
    int GetSerializationVersion() const
    {
        return 1;
    }
};

#endif
