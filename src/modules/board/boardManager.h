#pragma once
#include "board.h"

#include <vector>

#include "../../conf.h"
#include "../../mud.h"
#include "../../serializer.h"
#include "../modules.h"
#include <json/json.h>

#ifdef MODULE_BOARD

class BoardManager : public ISerializable
{
    std::vector<Board*> _boards;

  public:
    BoardManager();
    ~BoardManager();
    void AddBoard(Board* board);
    void GetBoards(std::vector<Board*>* boards);
    Board* GetBoardByIndex(int index);

    // JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);
    int GetSerializationVersion() const
    {
        return 1;
    }
};

#endif
