/*
 *The main board system.
 *Holds posts for a single board etc.
 */
#pragma once
#include "boardPost.h"

#include <string>
#include <vector>

#include "../../mud.h"
#include "../modules.h"
#include <json/json.h>

#ifdef MODULE_BOARD

class Board
{
    std::vector<BoardPost*> _posts;
    std::string _name;
    FLAG _access;

  public:
    Board();
    ~Board();
    void SetName(const std::string& name);
    std::string GetName() const;
    void SetAccess(FLAG access);
    FLAG GetAccess() const;
    void AddPost(BoardPost* post);
    std::vector<BoardPost*>* GetPosts();
    BoardPost* GetPostByIndex(int index);

    // JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);
    int GetSerializationVersion() const
    {
        return 1;
    }
};

#endif
