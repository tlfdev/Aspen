#include "scr_ObjectContainer.h"

#include "helper.h"
#include "scr_BaseObject.h"
#include "scr_Entity.h"
#include "script.h"

#include <angelscript.h>

#include <cassert>

#include <scriptarray.h>

#include "../entity.h"
#include "../mud.h"
#include "../objectContainer.h"

CScriptArray* GetObjectContents(Entity* obj)
{
    if (obj == nullptr)
    {
        return nullptr;
    }

    return ContainerToScriptArray<std::list<Entity*>>("array<Entity@>", *(obj->GetContents()));
}

void RegisterObjectContainerMethods(const char* obj)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    bool r;

    r = engine->RegisterMethod(obj, "array<Entity@>@ GetContents() const", asFUNCTION(GetObjectContents),
                               asCALL_CDECL_OBJLAST);
    assert(r);
    r = engine->RegisterMethod(obj, "bool CanReceive(Entity@ obj) const", asMETHOD(ObjectContainer, CanReceive));
    assert(r);
    r = engine->RegisterMethod(obj, "void ObjectEnter(Entity@ obj)", asMETHOD(ObjectContainer, ObjectEnter));
    assert(r);
    r = engine->RegisterMethod(obj, "void ObjectLeave(Entity@ obj)", asMETHOD(ObjectContainer, ObjectLeave));
    assert(r);
}
void InitializeObjectContainer()
{
    RegisterBaseObjectMethods("ObjectContainer");
    RegisterObjectContainerMethods("ObjectContainer");
}
