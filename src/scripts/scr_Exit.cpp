#include "script.h"

#include <angelscript.h>

#include <cassert>

#include "../exit.h"
#include "../mud.h"

void RegisterExitMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "vnum GetTo() const", asMETHOD(Exit, GetTo));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetTo(vnum)", asMETHOD(Exit, SetTo));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetName() const", asMETHOD(Exit, GetName));
    assert(r);
}
void InitializeExit()
{
    RegisterExitMethods("Exit");
}
