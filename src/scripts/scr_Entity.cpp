#include "scr_Entity.h"

#include "helper.h"
#include "scr_BaseObject.h"
#include "scr_ObjectContainer.h"
#include "script.h"

#include <angelscript.h>

#include <cassert>

#include "../entity.h"
#include "../mud.h"

void RegisterEntityMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "string& GetShort()", asMETHOD(Entity, GetShort));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetShort(string short)", asMETHOD(Entity, SetShort));
    assert(r);
    r = engine->RegisterMethod(obj, "ObjectContainer@ GetLocation()", asMETHOD(Entity, GetLocation));
    assert(r);
    r = engine->RegisterMethod(obj, "bool MoveTo(ObjectContainer@ obj)", asMETHOD(Entity, MoveTo));
    assert(r);
    r = engine->RegisterMethod(obj, "void SetLocation(ObjectContainer@ location)", asMETHOD(Entity, SetLocation));
    assert(r);
    r = engine->RegisterMethod(obj, "bool FromRoom()", asMETHOD(Entity, FromRoom));
    assert(r);
    r = engine->RegisterMethod(obj, "bool AddAlias(string alias)", asMETHOD(Entity, AddAlias));
    assert(r);
    r = engine->RegisterMethod(obj, "bool AliasExists(string name)", asMETHOD(Entity, AliasExists));
    assert(r);
}

void InitializeEntity()
{
    RegisterBaseObjectMethods("Entity");
    RegisterObjectContainerMethods("Entity");
    RegisterEntityMethods("Entity");
}
