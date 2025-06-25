#pragma once
#include "framework.h"

namespace Misc {
    void nullFunc() {}

    int True() {
        return 1;
    }

    int False() {
        return 0;
    }

    static inline void (*KickPlayerOG)(AGameSession*, AController*);
    static void KickPlayer(AGameSession*, AController*) {
        Log("KickPlayer Called!");
        return;
    }

    void (*DispatchRequestOG)(__int64 a1, unsigned __int64* a2, int a3);
    void DispatchRequest(__int64 a1, unsigned __int64* a2, int a3)
    {
        return DispatchRequestOG(a1, a2, 3);
    }

    void Hook() {
        //MH_CreateHook((LPVOID)(ImageBase + 0x100e038), True, nullptr); // collectgarbage
        MH_CreateHook((LPVOID)(ImageBase + 0x83f36d4), KickPlayer, (LPVOID*)&KickPlayerOG); // Kickplayer
        MH_CreateHook((LPVOID)(ImageBase + 0x16BBFE0), DispatchRequest, (LPVOID*)&DispatchRequestOG); // dont have the offset

        MH_CreateHook((LPVOID)(ImageBase + 0x4834b50), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0xd4fc70), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0xdbd0b4), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0xdbe928), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0xdbdeb4), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0x18aca00), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0x6eb0d6c), nullFunc, nullptr);

        // No FortLocalPlayer patch
        MH_CreateHook((LPVOID)(ImageBase + 0x426A2D4), nullFunc, nullptr);
        MH_CreateHook((LPVOID)(ImageBase + 0x10B93F4), nullFunc, nullptr);

        Log("Misc Hooked!");
    }
}