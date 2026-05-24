#include "script.h"

#include "scr_BaseObject.h"
#include "scr_Entity.h"
#include "scr_Exit.h"
#include "scr_Living.h"
#include "scr_ObjectContainer.h"
#include "scr_Player.h"
#include "scr_Room.h"

#include <angelscript.h>

#include <cassert>
#include <sstream>

#include <scriptarray.h>
#include <scriptdictionary.h>
#include <scriptmath.h>
#include <scriptstdstring.h>

#include "../event.h"
#include "../log.h"
#include "../mud.h"
#include "../world.h"

static void MessageCallback(const asSMessageInfo* msg, void* param)
{
    std::stringstream st;

    // error type:
    switch (msg->type)
    {
        case asMSGTYPE_ERROR:
            st << "[ERROR]";
            break;
        case asMSGTYPE_WARNING:
            st << "[WARNING]";
            break;
        case asMSGTYPE_INFORMATION:
            st << "[INFORMATION]";
            break;
        default:
            st << "[UNKNOWN]";
    }

    st << " @" << msg->section << ":";
    st << "LN " << msg->row << ", COL " << msg->col << ":";
    st << msg->message;
    WriteLog(st.str());
}

ScriptEngine* ScriptEngine::_ptr;
ScriptEngine::ScriptEngine()
{
    _engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    RegisterScriptArray(_engine, true);
    RegisterScriptMath(_engine);
    RegisterStdString(_engine);
    RegisterStdStringUtils(_engine);
    RegisterScriptDictionary(_engine);
    _engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
}
ScriptEngine::~ScriptEngine()
{
    _engine->Release();
}

void ScriptEngine::Initialize()
{
    _ptr = new ScriptEngine();
}
void ScriptEngine::Release()
{
    if (_ptr)
    {
        delete _ptr;
        _ptr = nullptr;
    }
}
ScriptEngine* ScriptEngine::GetPtr()
{
    return _ptr;
}

asIScriptEngine* ScriptEngine::GetBaseEngine()
{
    return _engine;
}
bool ScriptEngine::RegisterMethod(const char* obj, const char* decl, asSFuncPtr ptr)
{
    int ret = 0;
    ret = _engine->RegisterObjectMethod(obj, decl, ptr, asCALL_THISCALL);
    return (ret >= 0 ? true : false);
}
bool ScriptEngine::RegisterMethod(const char* obj, const char* decl, asSFuncPtr ptr, asDWORD callConv)
{
    int ret = 0;
    ret = _engine->RegisterObjectMethod(obj, decl, ptr, callConv);
    return (ret >= 0 ? true : false);
}
bool ScriptEngine::RegisterObject(const char* obj)
{
    int r = 0;

    r = _engine->RegisterObjectType(obj, 0, asOBJ_REF | asOBJ_NOCOUNT);
    return (r >= 0 ? true : false);
}

CEVENT(ScriptEngine, Shutdown)
{
    WriteLog("Cleaning up scripting.");
    ScriptEngine::Release();
}

/**
we initialize individual objects here.
This needs to be done separate from the methods,
as methods sometimes rely on other objects.
*/
static void InitializeObjects()
{
    bool ret = false;
    ScriptEngine* engine = ScriptEngine::GetPtr();

    assert(engine->GetBaseEngine()->RegisterTypedef("vnum", "int") >= 0);
    assert(engine->GetBaseEngine()->RegisterTypedef("flag", "uint") >= 0);
    ret = engine->RegisterObject("BaseObject");
    assert(ret);
    ret = engine->RegisterObject("Entity");
    assert(ret);
    ret = engine->RegisterObject("ObjectContainer");
    assert(ret);
    ret = engine->RegisterObject("Exit");
    assert(ret);
    ret = engine->RegisterObject("Room");
    assert(ret);
    ret = engine->RegisterObject("Living");
    assert(ret);
    ret = engine->RegisterObject("Player");
    assert(ret);
}

/**
Initializes individual methods, properties and etc on specific objects.
*/
static void InitializeObjectTraits()
{
    InitializeBaseObject();
    InitializeEntity();
    InitializeObjectContainer();
    InitializeExit();
    InitializeRoom();
    InitializeLiving();
    InitializePlayerScript();
}

bool InitializeScript()
{
    World* world = World::GetPtr();
    ScriptEngine* engine = nullptr;

    WriteLog("Initializing scripting.");

    ScriptEngine::Initialize();
    engine = ScriptEngine::GetPtr();
    world->events.AddCallback("Shutdown",
                              std::bind(&ScriptEngine::Shutdown, engine, std::placeholders::_1, std::placeholders::_2));

    InitializeObjects();
    InitializeObjectTraits();

    return true;
}
