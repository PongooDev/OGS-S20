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

		OnPawnAISpawnedOG(Controller, Pawn);

		AFortAthenaAIBotController* PC = (AFortAthenaAIBotController*)Pawn->Controller;
		auto PlayerState = (AFortPlayerStateAthena*)Pawn->PlayerState;

		PC->RunBehaviorTree(PC->BehaviorTree);

		for (auto SkillSet : PC->BotSkillSetClasses)
		{
			if (!SkillSet)
				continue;

			if (auto AimingSkill = Cast<UFortAthenaAIBotAimingDigestedSkillSet>(SkillSet))
				PC->CacheAimingDigestedSkillSet = AimingSkill;

			if (auto AttackingSkill = Cast<UFortAthenaAIBotAttackingDigestedSkillSet>(SkillSet))
				PC->CacheAttackingSkillSet = AttackingSkill;

			if (auto HarvestSkill = Cast<UFortAthenaAIBotHarvestDigestedSkillSet>(SkillSet))
				PC->CacheHarvestDigestedSkillSet = HarvestSkill;

			if (auto InventorySkill = Cast<UFortAthenaAIBotInventoryDigestedSkillSet>(SkillSet))
				PC->CacheInventoryDigestedSkillSet = InventorySkill;

			if (auto LootingSkill = Cast<UFortAthenaAIBotLootingDigestedSkillSet>(SkillSet))
				PC->CacheLootingSkillSet = LootingSkill;

			if (auto MovementSkill = Cast<UFortAthenaAIBotMovementDigestedSkillSet>(SkillSet))
				PC->CacheMovementSkillSet = MovementSkill;

			if (auto PerceptionSkill = Cast<UFortAthenaAIBotPerceptionDigestedSkillSet>(SkillSet))
				PC->CachePerceptionDigestedSkillSet = PerceptionSkill;

			if (auto PlayStyleSkill = Cast<UFortAthenaAIBotPlayStyleDigestedSkillSet>(SkillSet))
				PC->CachePlayStyleSkillSet = PlayStyleSkill;

			if (auto RangeAttackSkill = Cast<UFortAthenaAIBotRangeAttackDigestedSkillSet>(SkillSet))
				PC->CacheRangeAttackSkillSet = RangeAttackSkill;

			if (auto UnstuckSkill = Cast<UFortAthenaAIBotUnstuckDigestedSkillSet>(SkillSet))
				PC->CacheUnstuckSkillSet = UnstuckSkill;
		}

		if (PC->CosmeticLoadoutBC.Character)
		{
			if (PC->CosmeticLoadoutBC.Character->HeroDefinition)
			{
				for (int i = 0; i < PC->CosmeticLoadoutBC.Character->HeroDefinition->Specializations.Num(); i++)
				{
					auto SpecStr = UKismetStringLibrary::Conv_NameToString(PC->CosmeticLoadoutBC.Character->HeroDefinition->Specializations[i].ObjectID.AssetPathName);
					UFortHeroSpecialization* Spec = StaticLoadObject<UFortHeroSpecialization>(SpecStr.ToString());
					if (Spec)
					{
						for (int j = 0; j < Spec->CharacterParts.Num(); j++)
						{
							auto PartStr = UKismetStringLibrary::Conv_NameToString(Spec->CharacterParts[j].ObjectID.AssetPathName);
							UCustomCharacterPart* CharacterPart = StaticLoadObject<UCustomCharacterPart>(PartStr.ToString());
							if (CharacterPart)
							{
								//PlayerState->CharacterData.Parts[(uintptr_t)CharacterPart->CharacterPartType] = CharacterPart;
								Pawn->ServerChoosePart(CharacterPart->CharacterPartType, CharacterPart); //try this??
							}

							PartStr.Free();
							Log("PartStr: " + PartStr.ToString()); //we need the string ofc
						}
					}

					SpecStr.Free();
				}
			}
		}
		PlayerState->OnRep_CharacterData();

		if (!PC->Inventory)
			PC->Inventory = SpawnActor<AFortInventory>({}, {}, PC);

		auto BlackboardComp = ((AFortAthenaAIBotController*)Pawn->Controller)->Blackboard;
		static auto name1 = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep");
		static auto name1b = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhase");
		BlackboardComp->SetValueAsEnum(name1, 7);
		BlackboardComp->SetValueAsEnum(name1b, (uint8)EAthenaGamePhase::SafeZones);

		UFortWeaponMeleeItemDefinition* PickDef = StaticLoadObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		if (PickDef) {
			UFortWorldItem* Item = (UFortWorldItem*)PickDef->CreateTemporaryItemInstanceBP(1, 0);
			Item->OwnerInventory = PC->Inventory;
			Item->ItemEntry.LoadedAmmo = 1;
			PC->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
			PC->Inventory->Inventory.ItemInstances.Add(Item);
			PC->Inventory->Inventory.MarkItemDirty(Item->ItemEntry);
			PC->Inventory->HandleInventoryLocalUpdate();
		}
		else {
			Log("Default Pickaxe dont exist!");
		}
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