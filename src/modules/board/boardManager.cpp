#include "boardManager.h"

#include "board.h"

#include <vector>

#include "../../conf.h"
#include "../../mud.h"
#include "../../serializationHelpers.hpp"
#include "../modules.h"

#ifdef MODULE_BOARD

BoardManager::BoardManager() {}
BoardManager::~BoardManager()
{
    std::vector<Board*>::iterator it, itEnd;

    itEnd = _boards.end();
    for (it = _boards.begin(); it != itEnd; ++it)
    {
        delete (*it);
    }
}

void BoardManager::AddBoard(Board* board)
{
    _boards.push_back(board);
}
void BoardManager::GetBoards(std::vector<Board*>* boards)
{
    std::vector<Board*>::iterator it, itEnd;

    itEnd = _boards.end();
    for (it = _boards.begin(); it != itEnd; ++it)
    {
        boards->push_back((*it));
    }
}
Board* BoardManager::GetBoardByIndex(int index)
{
    if ((index < 1) || (index > (int)_boards.size()))
    {
        return NULL;
    }

    return _boards[index - 1];
}

// JSON serialization
void BoardManager::ToJson(Json::Value& json) const
{
    // Serialize boards
    Json::Value boardsArray(Json::arrayValue);
    for (const auto* board : _boards)
    {
        if (board)
        {
            Json::Value boardJson;
            board->ToJson(boardJson);
            boardsArray.append(boardJson);
        }
    }
    json["boards"] = boardsArray;
}

void BoardManager::FromJson(const Json::Value& json, int version)
{
    // Deserialize boards
    if (json.isMember("boards") && json["boards"].isArray())
    {
        const Json::Value& boardsArray = json["boards"];
        for (const auto& boardJson : boardsArray)
        {
            Board* board = new Board();
            board->FromJson(boardJson, version);
            _boards.push_back(board);
        }
    }
}
#endif
