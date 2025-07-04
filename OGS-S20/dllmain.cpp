#include "framework.h"
#include "GameMode.h"
#include "PC.h"
#include "Abilities.h"
#include "Inventory.h"
#include "Building.h"
#include "Looting.h"

#include "Misc.h"
#include "Net.h"
#include "Tick.h"

#include "PE.h"

#include "Globals.h"

void InitConsole() {
    AllocConsole();
    FILE* fptr;
    freopen_s(&fptr, "CONOUT$", "w+", stdout);
    SetConsoleTitleA("OGS 20.40 | Starting...");
    Log("Welcome to OGS, Made with love by ObsessedTech!");
}

void LoadWorld() {
    Log("Loading World!");
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    if (!Globals::bCreativeEnabled && !Globals::bSTWEnabled) {
        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Artemis_Terrain", nullptr);
    }
    else if (Globals::bCreativeEnabled) {
        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Creative_NoApollo_Terrain", nullptr);
    }
}

void Hook() {
    GameMode::Hook();
    PC::Hook();
    Abilities::Hook();
    Inventory::Hook();
    Building::Hook();
    Looting::Hook();

    Misc::Hook();
    Net::Hook();
    Tick::Hook();

    PE::Hook();

    Sleep(1000);
    MH_EnableHook(MH_ALL_HOOKS);
}

static void WaitForLogin() {
    Log("Waiting for login!");

    FName Frontend = UKismetStringLibrary::Conv_StringToName(L"Frontend");
    FName MatchState = UKismetStringLibrary::Conv_StringToName(L"InProgress");

    while (true) {
        UWorld* CurrentWorld = ((UWorld*)UWorld::GetWorld());
        if (CurrentWorld) {
            if (CurrentWorld->Name == Frontend) {
                auto GameMode = (AGameMode*)CurrentWorld->AuthorityGameMode;
                if (GameMode->GetMatchState() == MatchState) {
                    break;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
    Log("Logged In!");
}

DWORD Main(LPVOID) {
    InitConsole();
    MH_Initialize();
    Log("MinHook Initialised!");

    while (UEngine::GetEngine() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    WaitForLogin();

    Hook();

    *(bool*)(InSDKUtils::GetImageBase() + 0xc425a74) = false; //GIsClient
    *(bool*)(InSDKUtils::GetImageBase() + 0xc425a74 + 1) = true; //GIsServer

    Sleep(1000);
    LoadWorld();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}