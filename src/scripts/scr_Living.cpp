#include "scr_Living.h"

#include "scr_BaseObject.h"
#include "scr_Entity.h"
#include "scr_ObjectContainer.h"
#include "script.h"

#include <angelscript.h>

#include <cassert>

#include "../living.h"
#include "../mud.h"

void RegisterLivingMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "bool IsLiving() const", asMETHOD(Living, IsLiving));
    assert(r);
    r = engine->RegisterMethod(obj, "unsigned int GetPosition() const", asMETHOD(Living, GetPosition));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetPosition(unsigned int pos)", asMETHOD(Living, SetPosition));
    assert(r);
}

void InitializeLiving()
{
    RegisterBaseObjectMethods("Living");
    RegisterObjectContainerMethods("Living");
    RegisterEntityMethods("Living");
    RegisterLivingMethods("Living");
}
