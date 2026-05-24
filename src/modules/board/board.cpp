#include "board.h"

#include "boardPost.h"

#include <string>
#include <vector>

#include "../../conf.h"
#include "../../mud.h"
#include "../../serializationHelpers.hpp"

#ifdef MODULE_BOARD
Board::Board() {}
Board::~Board()
{
    std::vector<BoardPost*>::iterator it, itEnd;

    itEnd = _posts.end();
    for (it = _posts.begin(); it != itEnd; ++it)
    {
        delete (*it);
    }
}

void Board::SetName(const std::string& name)
{
    _name = name;
}
std::string Board::GetName() const
{
    return _name;
}

void Board::SetAccess(FLAG access)
{
    _access = access;
}
FLAG Board::GetAccess() const
{
    return _access;
}

void Board::AddPost(BoardPost* post)
{
    _posts.push_back(post);
}
std::vector<BoardPost*>* Board::GetPosts()
{
    return &_posts;
}
BoardPost* Board::GetPostByIndex(int index)
{
    if ((index < 1) || (index > (int)_posts.size()))
    {
        return NULL;
    }

    return _posts[index - 1];
}
// JSON serialization
void Board::ToJson(Json::Value& json) const
{
    json["name"] = _name;
    json["access"] = _access;

    // Serialize posts
    Json::Value postsArray(Json::arrayValue);
    for (const auto* post : _posts)
    {
        if (post)
        {
            Json::Value postJson;
            post->ToJson(postJson);
            postsArray.append(postJson);
        }
    }
    json["posts"] = postsArray;
}

void Board::FromJson(const Json::Value& json, int version)
{
    using namespace JsonSerializerHelpers;

    _name = GetString(json, "name", "");
    _access = GetUInt(json, "access", 0);

    // Deserialize posts
    if (json.isMember("posts") && json["posts"].isArray())
    {
        const Json::Value& postsArray = json["posts"];
        for (const auto& postJson : postsArray)
        {
            BoardPost* post = new BoardPost();
            post->FromJson(postJson, version);
            _posts.push_back(post);
        }
    }
}
#endif
