#pragma once
/*
 *Script helpers.
 *These are the templated functions, do not include directly.
 */
#include "script.h"

#include <angelscript.h>

#include <scriptarray.h>


template <class C>
CScriptArray* ContainerToScriptArray(const char* odecl, C& container)
{
    ScriptEngine* engine = ScriptEngine::GetPtr();
    size_t i = 0;        // for index.
    void* ptr = nullptr; // holds the individual element.
    CScriptArray* ret = nullptr;

    const auto size = container.size();
    auto objtype = engine->GetBaseEngine()->GetTypeInfoByDecl(odecl);
    ret = CScriptArray::Create(objtype, size);

    // we do the actual copy.
    for (auto it : container)
    {
        ptr = &it;
        ret->SetValue(i, ptr);
        i++;
    }

    return ret;
}
