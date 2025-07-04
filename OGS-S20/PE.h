#pragma once
#include "framework.h"

namespace PE {
    static void* (*ProcessEventOG)(UObject*, UFunction*, void*);
    void* ProcessEvent(UObject* Obj, UFunction* Function, void* Params)
    {
        if (Function) {

        }

        return ProcessEventOG(Obj, Function, Params);
    }

    void Hook() {
        MH_CreateHook((LPVOID)(ImageBase + SDK::Offsets::ProcessEvent), ProcessEvent, (LPVOID*)&ProcessEventOG);

        Log("PE Hooked!");
    }
}