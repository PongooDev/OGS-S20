#pragma once
#include "framework.h"

namespace Bots {
	static std::vector<UAthenaCharacterItemDefinition*> CIDs{};
	static std::vector<UAthenaPickaxeItemDefinition*> Pickaxes{};
	static std::vector<UAthenaBackpackItemDefinition*> Backpacks{};
	static std::vector<UAthenaGliderItemDefinition*> Gliders{};
	static std::vector<UAthenaSkyDiveContrailItemDefinition*> Contrails{};
	inline std::vector<UAthenaDanceItemDefinition*> Dances{};

	// Pathfinding
	inline void (*InitializeForWorldOG)(UNavigationSystemV1* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode);
	void InitializeForWorld(UNavigationSystemV1* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode)
	{
		Log("InitialiseForWorld: " + World->GetName() + " For NavSystem: " + NavSystem->GetName());
		auto AthenaNavSystem = (UAthenaNavSystem*)NavSystem;
		AthenaNavSystem->bAutoCreateNavigationData = true;
		return InitializeForWorldOG(NavSystem, World, Mode);
	}

	inline void (*OnPawnAISpawnedOG)(AActor* Controller, AFortPlayerPawnAthena* Pawn);
	void OnPawnAISpawned(AActor* Controller, AFortPlayerPawnAthena* Pawn)
	{
		Log("OnPawnAISpawned!");
		AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		return OnPawnAISpawnedOG(Controller, Pawn);
	}

	inline void (*InventoryBaseOnSpawnedOG)(UFortAthenaAISpawnerDataComponent_InventoryBase* a1, APawn* a2);
	void InventoryBaseOnSpawned(UFortAthenaAISpawnerDataComponent_InventoryBase* a1, APawn* Pawn)
	{
		if (!Pawn || !Pawn->Controller)
			return;
		auto PC = (AFortAthenaAIBotController*)Pawn->Controller;

		if (!PC->Inventory)
			PC->Inventory = SpawnActor<AFortInventory>({}, {}, PC);

		InventoryBaseOnSpawnedOG(a1, Pawn);
	}

	void (*OnPossessedPawnDiedOG)(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum);
	void OnPossessedPawnDied(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum)
	{
		if (!PC)
			return;

		return OnPossessedPawnDiedOG(PC, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, HitComp, BoneName, Momentum);
	}

	void Hook() {
		// Dont think this is right vtable offset for this but it has the athenanavsystem and i think thats all we need
		HookVTable(UAthenaNavSystem::GetDefaultObj(), 88, InitializeForWorld, (LPVOID*)&InitializeForWorldOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x631C04C), OnPawnAISpawned, (LPVOID*)&OnPawnAISpawnedOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x70A86B0), InventoryBaseOnSpawned, (LPVOID*)&InventoryBaseOnSpawnedOG);

		//MH_CreateHook((LPVOID)(ImageBase + 0x631C8C8), OnPossessedPawnDied, (LPVOID*)&OnPossessedPawnDiedOG);

		Log("Bots Hooked!");
	}
}