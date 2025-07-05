#pragma once
#include "framework.h"

namespace Bots {
	// Pathfinding
	inline void (*InitializeForWorldOG)(UNavigationSystemV1* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode);
	void InitializeForWorld(UNavigationSystemV1* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode)
	{
		Log("InitialiseForWorld: " + World->GetName() + " For NavSystem: " + NavSystem->GetName());
		auto AthenaNavSystem = (UAthenaNavSystem*)NavSystem;
		AthenaNavSystem->bAutoCreateNavigationData = true;
		return InitializeForWorldOG(NavSystem, World, Mode);
	}

	void Hook() {
		HookVTable(UAthenaNavSystem::GetDefaultObj(), 0x55, InitializeForWorld, (LPVOID*)&InitializeForWorldOG);

		Log("Bots Hooked!");
	}
}