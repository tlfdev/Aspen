#include "helper.h"
#include "scr_BaseObject.h"
#include "scr_ObjectContainer.h"
#include "script.h"

#include <angelscript.h>

#include <cassert>

#include <scriptarray.h>

#include "../mud.h"
#include "../room.h"

CScriptArray* GetRoomExits(Room* obj)
{
    if (obj == nullptr)
    {
        return nullptr;
    }

    return ContainerToScriptArray<std::vector<Exit*>>("array<Exit@>", *(obj->GetExits()));
}

void RegisterRoomMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "bool AddExit(Exit@ exit)", asMETHOD(Room, AddExit));
    assert(r);
    r = engine->RegisterMethod(obj, "bool ExitExists(int dir)", asMETHOD(Room, ExitExists));
    assert(r);
    r = engine->RegisterMethod(obj, "Exit@ GetExit(int dir)", asMETHOD(Room, GetExit));
    assert(r);
    r = engine->RegisterMethod(obj, "array<Exit@>@ GetExits()", asFUNCTION(GetRoomExits), asCALL_CDECL_OBJLAST);
    assert(r);
    r = engine->RegisterMethod(obj, "void SetRoomFlag(flag)", asMETHOD(Room, SetRoomFlag));
    assert(r);
    r = engine->RegisterMethod(obj, "flag GetRoomFlag()", asMETHOD(Room, GetRoomFlag));
    assert(r);
    r = engine->RegisterMethod(obj, "void TellAll(string message)", asMETHOD(Room, TellAll));
    assert(r);
    r = engine->RegisterMethod(obj, "void TellAllBut(string message, Player@ exclude)",
                               asMETHODPR(Room, TellAllBut, (const std::string&, Player*), void));
    assert(r);
    r = engine->RegisterMethod(obj, "string& TellObviousExits()", asMETHOD(Room, TellObviousExits));
    assert(r);
}

void InitializeRoom()
{
    RegisterBaseObjectMethods("Room");
    RegisterObjectContainerMethods("Room");
    RegisterRoomMethods("Room");
}
