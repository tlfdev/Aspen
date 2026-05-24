#include "scr_BaseObject.h"

#include "script.h"

#include <angelscript.h>

#include <cassert>

#include "../baseObject.h"
#include "../mud.h"

void RegisterBaseObjectMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "string& GetName()", asMETHOD(BaseObject, GetName));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetName(string name)", asMETHOD(BaseObject, SetName));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetDescription()", asMETHOD(BaseObject, GetDescription));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetDescription(string description)", asMETHOD(BaseObject, SetDescription));
    assert(r);
    // TODO: add get/set Zone
    r = engine->RegisterMethod(obj, "vnum GetOnum() const", asMETHOD(BaseObject, GetOnum));
    assert(r);
    // TODO: add components.
    r = engine->RegisterMethod(obj, "void Attach()", asMETHOD(BaseObject, Attach));
    assert(r);
    r = engine->RegisterMethod(obj, "bool IsPlayer() const", asMETHOD(BaseObject, IsPlayer));
    assert(r);
    r = engine->RegisterMethod(obj, "bool IsLiving() const", asMETHOD(BaseObject, IsLiving));
    assert(r);
    r = engine->RegisterMethod(obj, "bool IsRoom() const", asMETHOD(BaseObject, IsRoom));
    assert(r);
    r = engine->RegisterMethod(obj, "bool IsNpc() const", asMETHOD(BaseObject, IsNpc));
    assert(r);
    r = engine->RegisterMethod(obj, "bool IsObject() const", asMETHOD(BaseObject, IsObject));
    assert(r);
    r = engine->RegisterMethod(obj, "string& GetScript() const", asMETHOD(BaseObject, GetScript));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetScript(string script)", asMETHOD(BaseObject, SetScript));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetOnum(vnum num)", asMETHOD(BaseObject, SetOnum));
    assert(r);
    r = engine->RegisterMethod(obj, "bool HasComponent(string name)", asMETHOD(BaseObject, HasComponent));
    assert(r);
}

void InitializeBaseObject()
{
    // todo: add global properties (property, etc).
    RegisterBaseObjectMethods("BaseObject");
}
