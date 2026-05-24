/**
 * Contains registration logic and wrappers for the player object.
 */
#include "scr_Player.h"

#include "scr_BaseObject.h"
#include "scr_Entity.h"
#include "scr_Living.h"
#include "scr_ObjectContainer.h"
#include "script.h"

#include <angelscript.h>

#include <cassert>

#include "../mud.h"
#include "../player.h"

void RegisterPlayerMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "bool IsPlayer() const", asMETHOD(Player, IsPlayer));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetShort()", asMETHOD(Player, GetShort));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetTitle() const", asMETHOD(Player, GetTitle));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetTitle(string title)", asMETHOD(Player, SetTitle));
    assert(r);
    r = engine->RegisterMethod(obj, "flag GetRank() const", asMETHOD(Player, GetRank));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetRank(flag rank)", asMETHOD(Player, SetRank));
    assert(r);
    r = engine->RegisterMethod(obj, "flag GetPflag() const", asMETHOD(Player, GetPflag));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetPflag(flag flag)", asMETHOD(Player, SetPflag));
    assert(r);
    r = engine->RegisterMethod(obj, "unsigned int GetOnlineTime() const", asMETHOD(Player, GetOnlineTime));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetOnlineTime(unsigned int time)", asMETHOD(Player, SetOnlineTime));
    assert(r);
    r = engine->RegisterMethod(obj, "unsigned int GetFirstLogin() const", asMETHOD(Player, GetFirstLogin));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetFirstLogin(unsigned int first)", asMETHOD(Player, SetFirstLogin));
    assert(r);
    r = engine->RegisterMethod(obj, "unsigned int GetLastLogin() const", asMETHOD(Player, GetLastLogin));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetLastLogin(unsigned int last)", asMETHOD(Player, SetLastLogin));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetPrompt() const", asMETHOD(Player, GetPrompt));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetPrompt(string prompt)", asMETHOD(Player, SetPrompt));
    assert(r);
    r = engine->RegisterMethod(obj, "void Write(string text) const", asMETHOD(Player, Write));
    assert(r);
    r = engine->RegisterMethod(obj, "bool HasAccess(flag access) const", asMETHOD(Player, HasAccess));
    assert(r);
}

void InitializePlayerScript()
{
    RegisterBaseObjectMethods("Player");
    RegisterObjectContainerMethods("Player");
    RegisterEntityMethods("Player");
    RegisterLivingMethods("Player");
    RegisterPlayerMethods("Player");
}
