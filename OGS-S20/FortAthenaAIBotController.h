#pragma once
#include "framework.h"
#include "PhoebeDisplayNames.h"

#include "NPCs.h"

namespace FortAthenaAIBotController {
	struct BotSpawnData {
		UClass* BotSpawnerData;
		int32 RequestID;
		FString BotIDSuffix;

		AFortAthenaAIBotController* Controller;
		AFortPlayerPawnAthena* Pawn;
		AFortPlayerStateAthena* PlayerState;
	};
	std::vector<BotSpawnData> SpawnedBots;

	void (*CreateAndConfigureNavigationSystemOG)(UAthenaNavSystemConfig* ModuleConfig, UWorld* World);
	void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* ModuleConfig, UWorld* World)
	{
		Log("CreateAndConfigureNavigationSystem For World: " + World->GetName() + " For NavConfig: " + ModuleConfig->GetName());
		ModuleConfig->bPrioritizeNavigationAroundSpawners = true;
		ModuleConfig->bAutoSpawnMissingNavData = true;
		ModuleConfig->bSpawnNavDataInNavBoundsLevel = true;
		return CreateAndConfigureNavigationSystemOG(ModuleConfig, World);
	}

	// Pathfinding
	void (*InitializeForWorldOG)(UNavigationSystemV1* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode);
	void InitializeForWorld(UAthenaNavSystem* NavSystem, UWorld* World, EFNavigationSystemRunMode Mode)
	{
		Log("InitialiseForWorld: " + World->GetName() + " For NavSystem: " + NavSystem->GetName());
		NavSystem->bAutoCreateNavigationData = true;
		AthenaNavSystem = NavSystem;
		return InitializeForWorldOG(NavSystem, World, Mode);
	}

	inline void (*OnPawnAISpawnedOG)(AActor* Controller, AFortPlayerPawnAthena* Pawn);
	void OnPawnAISpawned(AActor* Controller, AFortPlayerPawnAthena* Pawn)
	{
		static int AmountTimesCalled = 0;

		AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		OnPawnAISpawnedOG(Controller, Pawn);
		if (!AthenaNavSystem->MainNavData) {
			Log("NavData Dont Exist!");
		}

		UClass* BotSpawnerData = nullptr;

		auto PC = (AFortAthenaAIBotController*)Pawn->Controller;
		auto PlayerState = (AFortPlayerStateAthena*)Pawn->PlayerState;
		for (BotSpawnData& SpawnedBot : SpawnedBots) {
			if (AmountTimesCalled == SpawnedBot.RequestID) {
				SpawnedBot.BotIDSuffix = PC->BotIDSuffix;
				SpawnedBot.Controller = PC;
				SpawnedBot.Pawn = Pawn;
				SpawnedBot.PlayerState = PlayerState;
				if (SpawnedBot.BotSpawnerData) {
					BotSpawnerData = SpawnedBot.BotSpawnerData;
				}
			}
		}
		if (BotSpawnerData) {
			UFortAthenaAIBotSpawnerData* SpawnerData = Cast<UFortAthenaAIBotSpawnerData>(BotSpawnerData->DefaultObject);
			if (!SpawnerData) {
				Log("No SpawnerData!");
				return;
			}

			UFortAthenaAISpawnerDataComponent_AIBotInventory* InventoryComponent = (UFortAthenaAISpawnerDataComponent_AIBotInventory*)SpawnerData->GetInventoryComponent();
			if (!PC->StartupInventory) {
				PC->StartupInventory = (UFortAthenaAIBotInventoryItems*)UGameplayStatics::GetDefaultObj()->SpawnObject(UFortAthenaAIBotInventoryItems::StaticClass(), GameMode);
			}
			if (InventoryComponent) {
				PC->StartupInventory->Items = InventoryComponent->Items;
			}

			UFortAthenaAISpawnerDataComponent_SkillsetBase* SkillSetBase = SpawnerData->GetSkillSetComponent();
			if (SkillSetBase) {
				PC->BotSkillSetClasses = SkillSetBase->GetSkillSets();
			}

			UFortAthenaAISpawnerDataComponent_ConversationBase* ConversationComp = SpawnerData->GetConversationComponent();

			AbilitySystemComponent::GrantAbilitySet((AFortPlayerController*)PC, StaticLoadObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer"));
		}

		if (!PC->PathFollowingComponent->MyNavData) {
			PC->PathFollowingComponent->MyNavData = AthenaNavSystem->MainNavData;
		}
		PC->PathFollowingComponent->OnNavDataRegistered(PC->PathFollowingComponent->MyNavData);
		PC->PathFollowingComponent->Activate(true);

		if (!PC->BrainComponent || !PC->BrainComponent->IsA(UBrainComponent::StaticClass()))
		{
			PC->BrainComponent = (UBrainComponent*)UGameplayStatics::SpawnObject(UBrainComponent::StaticClass(), PC);
			RegisterComponent(PC->BrainComponent, UWorld::GetWorld(), nullptr);
		}

		UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());
		if (AIServicePlayerBots) {
			Log("Good!");
			*(bool*)(__int64(AIServicePlayerBots) + 0x7b8) = true; // bCanActivateBrain
		}

		if (PC->BotIDSuffix.ToString() == "DEFAULT")
		{
			if (!PC->RunBehaviorTree(PC->BehaviorTree)) {
				Log("BehaviorTree Failed To Run For Pawn: " + Pawn->GetFullName());
			}
			else {
				Log("Ran BehaviorTree: " + PC->BehaviorTree->GetFullName());
				PC->BlueprintOnBehaviorTreeStarted();
			}

			PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep"), (int)GameState->GamePhaseStep);
			PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_GamePhase"), (int)GameState->GamePhase);
			PC->Blackboard->SetValueAsBool(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_IsMovementBlocked"), false);
			PC->Blackboard->SetValueAsBool(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey"), false);
			if (BuildingFoundations.Num() > 0) {
				AActor* DropZone = BuildingFoundations[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, BuildingFoundations.Num() - 1)];
				if (DropZone) {
					PC->Blackboard->SetValueAsVector(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_JumpOffBus_Destination"), DropZone->K2_GetActorLocation());
				}
			}
			else {
				Log("No building foundations!");
			}

			auto BotPlayerState = (AFortPlayerStateAthena*)Pawn->PlayerState;
			if (!Characters.empty()) {
				auto CID = Characters[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Characters.size() - 1)];
				if (CID->HeroDefinition)
				{
					if (CID->HeroDefinition->Specializations.IsValid())
					{
						for (size_t i = 0; i < CID->HeroDefinition->Specializations.Num(); i++)
						{
							UFortHeroSpecialization* Spec = StaticLoadObject<UFortHeroSpecialization>(UKismetStringLibrary::GetDefaultObj()->Conv_NameToString(CID->HeroDefinition->Specializations[i].ObjectID.AssetPathName).ToString());
							if (Spec)
							{
								for (size_t j = 0; j < Spec->CharacterParts.Num(); j++)
								{
									UCustomCharacterPart* Part = StaticLoadObject<UCustomCharacterPart>(UKismetStringLibrary::GetDefaultObj()->Conv_NameToString(Spec->CharacterParts[j].ObjectID.AssetPathName).ToString());
									if (Part)
									{
										BotPlayerState->CharacterData.Parts[(uintptr_t)Part->CharacterPartType] = Part;
									}
								}
							}
						}
					}
				}
				if (CID) {
					Pawn->CosmeticLoadout.Character = CID;
				}
			}
			if (!Backpacks.empty() && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.5)) { // less likely to equip than skin cause lots of ppl prefer not to use backpack
				auto Backpack = Backpacks[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Backpacks.size() - 1)];
				for (size_t j = 0; j < Backpack->CharacterParts.Num(); j++)
				{
					UCustomCharacterPart* Part = Backpack->CharacterParts[j];
					if (Part)
					{
						BotPlayerState->CharacterData.Parts[(uintptr_t)Part->CharacterPartType] = Part;
					}
				}
			}
			if (!Gliders.empty()) {
				auto Glider = Gliders[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Gliders.size() - 1)];
				Pawn->CosmeticLoadout.Glider = Glider;
			}
			if (!Contrails.empty() && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.95)) {
				auto Contrail = Contrails[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Contrails.size() - 1)];
				Pawn->CosmeticLoadout.SkyDiveContrail = Contrail;
			}
			BotPlayerState->OnRep_CharacterData();
			ApplyCharacterCustomization(BotPlayerState, Pawn);

			for (size_t i = 0; i < Dances.size(); i++)
			{
				Pawn->CosmeticLoadout.Dances.Add(Dances.at(i));
			}

			if (PhoebeDisplayNames.size() != 0) {
				std::srand(static_cast<unsigned int>(std::time(0)));
				int randomIndex = std::rand() % PhoebeDisplayNames.size();
				std::string rdName = PhoebeDisplayNames[randomIndex];
				PhoebeDisplayNames.erase(PhoebeDisplayNames.begin() + randomIndex);

				int size_needed = MultiByteToWideChar(CP_UTF8, 0, rdName.c_str(), (int)rdName.size(), NULL, 0);
				std::wstring wideString(size_needed, 0);
				MultiByteToWideChar(CP_UTF8, 0, rdName.c_str(), (int)rdName.size(), &wideString[0], size_needed);


				FString CVName = FString(wideString.c_str());
				GameMode->ChangeName(PC, CVName, true);

				BotPlayerState->OnRep_PlayerName();
			}

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

			if (!PC->Inventory)
				PC->Inventory = SpawnActor<AFortInventory>({}, {}, PC);

			for (auto& Items : ((AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode)->StartingItems)
			{
				if (!Items.Item)
					continue;
				UFortWorldItem* Item = Cast<UFortWorldItem>(Items.Item->CreateTemporaryItemInstanceBP(Items.Count, 0));
				Item->OwnerInventory = PC->Inventory;
				FFortItemEntry& Entry = Item->ItemEntry;
				PC->Inventory->Inventory.ReplicatedEntries.Add(Entry);
				PC->Inventory->Inventory.ItemInstances.Add(Item);
				PC->Inventory->Inventory.MarkItemDirty(Entry);
				PC->Inventory->HandleInventoryLocalUpdate();
			}

			for (auto& Items : PC->StartupInventory->Items)
			{
				if (!Items.Item)
					continue;
				if (Items.Item->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
				{
					static UFortWeaponMeleeItemDefinition* PickDef = StaticLoadObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");;
					if (!Pickaxes.empty()) {
						PickDef = Pickaxes[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Pickaxes.size() - 1)]->WeaponDefinition;
					}
					UFortWorldItem* Item = Cast<UFortWorldItem>(PickDef->CreateTemporaryItemInstanceBP(1, 0));
					Item->OwnerInventory = PC->Inventory;
					FFortItemEntry& Entry = Item->ItemEntry;
					Entry.LoadedAmmo = 1;
					PC->Inventory->Inventory.ReplicatedEntries.Add(Entry);
					PC->Inventory->Inventory.ItemInstances.Add(Item);
					PC->Inventory->Inventory.MarkItemDirty(Entry);
					PC->Inventory->HandleInventoryLocalUpdate();

					PC->PendingEquipWeapon = Item;
					Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid, Entry.TrackerGuid, false);
					continue;
				}
				UFortWorldItem* Item = Cast<UFortWorldItem>(Items.Item->CreateTemporaryItemInstanceBP(Items.Count, 0));
				Item->OwnerInventory = PC->Inventory;
				FFortItemEntry& Entry = Item->ItemEntry;
				Entry.LoadedAmmo = 1;
				PC->Inventory->Inventory.ReplicatedEntries.Add(Entry);
				PC->Inventory->Inventory.ItemInstances.Add(Item);
				PC->Inventory->Inventory.MarkItemDirty(Entry);
				PC->Inventory->HandleInventoryLocalUpdate();
			}

			GameMode->AliveBots.Add(PC);
			GameState->PlayerBotsLeft++;
			GameState->OnRep_PlayerBotsLeft();

			AmountTimesCalled++;
			return;
		}

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

		ApplyCharacterCustomization(PlayerState, Pawn);

		if (Globals::bBotsShouldUseManualTicking) {
			PC->BrainComponent->StopLogic(L"Manual Ticking Enabled!");
		}
		PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep"), (int)GameState->GamePhaseStep);
		PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_GamePhase"), (int)GameState->GamePhase);
		PC->Blackboard->SetValueAsBool(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_IsMovementBlocked"), false);
		PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_RangeAttack_ExecutionStatus"), (int)EExecutionStatus::ExecutionAllowed);

		if (!PC->Inventory)
			PC->Inventory = SpawnActor<AFortInventory>({}, {}, PC);

		if (PC->StartupInventory && PC->Inventory) {
			for (auto& Items : PC->StartupInventory->Items)
			{
				UFortItemDefinition* ItemDef = Items.Item;
				if (!ItemDef) {
					return;
				}

				UFortWorldItem* Item = (UFortWorldItem*)ItemDef->CreateTemporaryItemInstanceBP(Items.Count, 0);
				Item->OwnerInventory = PC->Inventory;
				Item->ItemEntry.LoadedAmmo = 60;
				PC->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
				PC->Inventory->Inventory.ItemInstances.Add(Item);
				PC->Inventory->Inventory.MarkItemDirty(Item->ItemEntry);
				PC->Inventory->HandleInventoryLocalUpdate();
				if (auto WeaponDef = Cast<UFortWeaponRangedItemDefinition>(Item->ItemEntry.ItemDefinition))
				{
					PC->PendingEquipWeapon = Item;
					Pawn->EquipWeaponDefinition(WeaponDef, Item->ItemEntry.ItemGuid, Item->ItemEntry.TrackerGuid, false);
				}
			}
		}
		else {
			Log("StartupInventory is nullptr!");
		}

		TArray<AActor*> PatrolPointProviders;
		UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), AFortAthenaPatrolPathPointProvider::StaticClass(), &PatrolPointProviders);
		for (AActor* PatrolPointProviderActor : PatrolPointProviders) {
			AFortAthenaPatrolPathPointProvider* PatrolPointProvider = (AFortAthenaPatrolPathPointProvider*)PatrolPointProviderActor;
			if (PatrolPointProvider->AssociatedPatrolPath && PatrolPointProvider->AssociatedPatrolPath->GetFullName().contains(PC->BotIDSuffix.ToString())) {
				Log("Found Patrol Path!");
				PC->CachedPatrollingComponent->SetPatrolPath(PatrolPointProvider->AssociatedPatrolPath, false);
				break;
			}
		}

		Npcs::NpcBot* Bot = new Npcs::NpcBot(PC, Pawn, PlayerState);
		Npcs::NpcBots.push_back(Bot);
		Bot->BT_NPC = Npcs::ConstructBehaviorTree();

		AmountTimesCalled++;
	}

	inline void (*InventoryBaseOnSpawnedOG)(UFortAthenaAISpawnerDataComponent_InventoryBase* InventoryBase, APawn* Pawn);
	void InventoryBaseOnSpawned(UFortAthenaAISpawnerDataComponent_InventoryBase* InventoryBase, APawn* Pawn)
	{
		if (!Pawn || !Pawn->Controller)
			return;
		auto PC = (AFortAthenaAIBotController*)Pawn->Controller;

		if (!PC->Inventory)
			PC->Inventory = SpawnActor<AFortInventory>({}, {}, PC);

		InventoryBaseOnSpawnedOG(InventoryBase, Pawn);
	}

	void (*OnPossessedPawnDiedOG)(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum);
	void OnPossessedPawnDied(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum)
	{
		if (!PC || !PC->Pawn || !PC->PlayerState) {
			return;
		}
		AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		AFortPlayerPawnAthena* Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

		AFortPlayerStateAthena* KillerState = (AFortPlayerStateAthena*)InstigatedBy->PlayerState;

		static auto Bars = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass())) {
				continue;
			}
			if (!((UFortWorldItemDefinition*)PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition)->bCanBeDropped) {
				continue;
			}
			auto Def = PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition;
			SpawnPickup(Def, 0, 0, PC->Pawn->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::BotElimination);
			UFortAmmoItemDefinition* AmmoDef = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Def)->GetAmmoWorldItemDefinition_BP();
			if (AmmoDef) {
				SpawnPickup(AmmoDef, AmmoDef->DropCount, 0, PC->Pawn->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::BotElimination);
			}
		}
		SpawnPickup(Bars, UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(10, 30), 0, PC->Pawn->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::BotElimination);

		if (PC->BotIDSuffix.ToString() == "DEFAULT") {
			FDeathInfo& DeathInfo = PlayerState->DeathInfo;

			DeathInfo.bDBNO = Pawn->bWasDBNOOnDeath;
			DeathInfo.DeathLocation = Pawn->K2_GetActorLocation();
			DeathInfo.DeathTags = Pawn->DeathTags;
			DeathInfo.Downer = KillerState ? KillerState : nullptr;
			AFortPawn* KillerPawn = KillerState ? KillerState->GetCurrentPawn() : nullptr;
			DeathInfo.Distance = (KillerPawn && Pawn) ? KillerPawn->GetDistanceTo(Pawn) : 0.f;
			DeathInfo.FinisherOrDowner = KillerState ? KillerState : nullptr;
			DeathInfo.DeathCause = PlayerState->ToDeathCause(DeathInfo.DeathTags, DeathInfo.bDBNO);
			DeathInfo.bInitialized = true;
			PlayerState->OnRep_DeathInfo();

			for (int i = 0; i < GameMode->AliveBots.Num(); i++) {
				AFortAthenaAIBotController* Controller = GameMode->AliveBots[i];
				if (Controller == PC) {
					GameMode->AliveBots.Remove(i);
				}
			}
			GameState->PlayerBotsLeft--;
			GameState->OnRep_PlayerBotsLeft();
		}

		return;
		return OnPossessedPawnDiedOG(PC, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, HitComp, BoneName, Momentum);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x19B0D00), CreateAndConfigureNavigationSystem, (LPVOID*)&CreateAndConfigureNavigationSystemOG);

		// Dont think this is right vtable offset for this but it has the athenanavsystem and i think thats all we need
		HookVTable(UAthenaNavSystem::GetDefaultObj(), 88, InitializeForWorld, (LPVOID*)&InitializeForWorldOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x631C04C), OnPawnAISpawned, (LPVOID*)&OnPawnAISpawnedOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x651A43C), InventoryBaseOnSpawned, (LPVOID*)&InventoryBaseOnSpawnedOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x631C8C8), OnPossessedPawnDied, (LPVOID*)&OnPossessedPawnDiedOG);

		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAthenaAIServiceBots VeryVerbose", nullptr);
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAthenaBots VeryVerbose", nullptr);
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogNavigation VeryVerbose", nullptr);
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogNavigationDataBuild VeryVerbose", nullptr);

		Log("Bots Hooked!");
	}
}