#pragma once
#include "framework.h"
#include "Inventory.h"
#include "BotSpawner.h"

namespace Controller {
	void (*ServerAcknowledgePossessionOG)(AFortPlayerControllerAthena* PC, APawn* Pawn);
	inline void ServerAcknowledgePossession(AFortPlayerControllerAthena* PC, APawn* Pawn)
	{
		PC->AcknowledgedPawn = Pawn;

		return ServerAcknowledgePossessionOG(PC, Pawn);
	}

	void (*ServerReadyToStartMatchOG)(AFortPlayerControllerAthena* PC);
	void ServerReadyToStartMatch(AFortPlayerControllerAthena* PC) {
		if (!PC) {
			Log("ServerReadyToStartMatch: No PC!");
			return;
		}

		AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

		static bool bSetupWorld = false;
		if (!bSetupWorld)
		{
			bSetupWorld = true;

			Looting::SpawnLlamas();
			Looting::DestroyFloorLootSpawners();

			BotSpawner::SpawnBosses();
			BotSpawner::SpawnGuards();
			BotSpawner::SpawnNpcs();

			Log("Setup World!");
		}

		AbilitySystemComponent::InitAbilitiesForPlayer(PC);

		PlayerState->SquadId = PlayerState->TeamIndex - 3;
		PlayerState->OnRep_SquadId();

		FGameMemberInfo Member;
		Member.MostRecentArrayReplicationKey = -1;
		Member.ReplicationID = -1;
		Member.ReplicationKey = -1;
		Member.TeamIndex = PlayerState->TeamIndex;
		Member.SquadId = PlayerState->SquadId;
		Member.MemberUniqueId = PlayerState->UniqueId;

		GameState->GameMemberInfoArray.Members.Add(Member);
		GameState->GameMemberInfoArray.MarkItemDirty(Member);

		static auto Bars = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");
		if (Bars) {
			Inventory::GiveItem(PC, Bars, UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, 5000), 0);
		}

		UAthenaGadgetItemDefinition* VictoryCrown = StaticLoadObject<UAthenaGadgetItemDefinition>("/VictoryCrownsGameplay/Items/AGID_VictoryCrown.AGID_VictoryCrown");
		if (VictoryCrown) {
			Inventory::GiveItem(PC, VictoryCrown, 1, 0);
			AbilitySystemComponent::GrantAbilitySet(PC, StaticLoadObject<UFortAbilitySet>("/VictoryCrownsGameplay/Items/AS_VictoryCrown.AS_VictoryCrown"));
		}

		return ServerReadyToStartMatchOG(PC);
	}

	inline void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Comp, FRotator Rotation)
	{
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		auto PC = (AFortPlayerControllerAthena*)Comp->GetOwner();
		UWorld::GetWorld()->AuthorityGameMode->RestartPlayer(PC);

		if (PC->MyFortPawn)
		{
			PC->ClientSetRotation(Rotation, true);
			PC->MyFortPawn->BeginSkydiving(true);
			PC->MyFortPawn->SetHealth(100);
			PC->MyFortPawn->SetShield(0);
		}

		if (PC && PC->WorldInventory)
		{
			for (int i = PC->WorldInventory->Inventory.ReplicatedEntries.Num() - 1; i >= 0; i--)
			{
				if (((UFortWorldItemDefinition*)PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition)->bCanBeDropped)
				{
					std::string Name = PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->Name.ToString();
					if (Name.contains("VictoryCrown") || Name.contains("Wads")) continue;

					int Count = PC->WorldInventory->Inventory.ReplicatedEntries[i].Count;
					Inventory::RemoveItem(PC, PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition, Count);
				}
			}
		}
	}

	inline void (*ServerAttemptInteractOG)(UFortControllerComponent_Interaction* Comp, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalData, EInteractionBeingAttempted InteractionBeingAttempted);
	inline void ServerAttemptInteract(UFortControllerComponent_Interaction* Comp, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalData, EInteractionBeingAttempted InteractionBeingAttempted)
	{
		if (!ReceivingActor || !Comp) {
			return;
		}

		ServerAttemptInteractOG(Comp, ReceivingActor, InteractComponent, InteractType, OptionalData, InteractionBeingAttempted);

		Log("ServerAttemptInteract: " + ReceivingActor->GetName());

		AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(Comp->GetOwner());
		if (!PC) {
			return ServerAttemptInteractOG(Comp, ReceivingActor, InteractComponent, InteractType, OptionalData, InteractionBeingAttempted);
		}

		if (PC->MyFortPawn && PC->MyFortPawn->IsInVehicle())
		{
			auto Vehicle = PC->MyFortPawn->BP_GetVehicle();

			if (Vehicle)
			{
				auto SeatIdx = PC->MyFortPawn->GetVehicleSeatIndex();
				Log(std::to_string(SeatIdx));
				auto WeaponComp = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());
				if (WeaponComp)
				{
					Inventory::GiveItem(PC, WeaponComp->WeaponSeatDefinitions[SeatIdx].VehicleWeapon, 1, 9999);
					for (size_t i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
					{
						if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == WeaponComp->WeaponSeatDefinitions[SeatIdx].VehicleWeapon)
						{
							PC->SwappingItemDefinition = PC->MyFortPawn->CurrentWeapon->WeaponData;
							PC->ServerExecuteInventoryItem(PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid);
							break;
						}
					}
				}
			}
		}
	}

	void ServerPlayEmoteItem(AFortPlayerControllerAthena* PC, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber) {
		Log("ServerPlayEmoteItem Called!");

		if (!PC || !EmoteAsset)
			return;

		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
		if (GameState->GamePhase == EAthenaGamePhase::Aircraft) {
			return;
		}

		UClass* DanceAbility = StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
		UClass* SprayAbility = StaticLoadObject<UClass>("/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");

		if (!DanceAbility || !SprayAbility)
			return;

		auto EmoteDef = (UAthenaDanceItemDefinition*)EmoteAsset;
		if (!EmoteDef)
			return;

		PC->MyFortPawn->bMovingEmote = EmoteDef->bMovingEmote;
		PC->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;
		PC->MyFortPawn->bMovingEmoteForwardOnly = EmoteDef->bMoveForwardOnly;
		PC->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;

		FGameplayAbilitySpec Spec{};
		UGameplayAbility* Ability = nullptr;

		if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
		{
			Ability = (UGameplayAbility*)SprayAbility->DefaultObject;
		}

		if (Ability == nullptr) {
			Ability = (UGameplayAbility*)DanceAbility->DefaultObject;
		}

		AbilitySpecConstructor(&Spec, Ability, 1, -1, EmoteDef);
		GiveAbilityAndActivateOnce(((AFortPlayerStateAthena*)PC->PlayerState)->AbilitySystemComponent, &Spec.Handle, Spec, nullptr);
	}

	void (*MovingEmoteStoppedOG)(AFortPawn* Pawn);
	void MovingEmoteStopped(AFortPawn* Pawn)
	{
		if (!Pawn)
			return;

		Pawn->bMovingEmote = false;
		Pawn->bMovingEmoteFollowingOnly = false;

		return MovingEmoteStoppedOG(Pawn);
	}

	void ServerReturnToMainMenu(AFortPlayerControllerAthena* PC)
	{
		PC->ClientReturnToMainMenu(L"");
	}

	void ServerCheat(AFortPlayerControllerAthena* PC, FString& Msg) {
		if (Globals::bIsProdServer)
			return;

		auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
		auto Math = (UKismetMathLibrary*)UKismetMathLibrary::StaticClass()->DefaultObject;
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		auto Statics = (UGameplayStatics*)UGameplayStatics::StaticClass()->DefaultObject;

		std::string Command = Msg.ToString();
		Log(Command);

		if (Command == "GodMode") {
			if (!PC->MyFortPawn->bIsInvulnerable) {
				PC->MyFortPawn->bIsInvulnerable = true;
			}
			else {
				PC->MyFortPawn->bIsInvulnerable = false;
			}
		}
		else if (Command == "DumpLoc") {
			FVector Loc = PC->Pawn->K2_GetActorLocation();
			Log("X: " + std::to_string(Loc.X));
			Log("Y: " + std::to_string(Loc.Y));
			Log("Z: " + std::to_string(Loc.Z));
		}
		else if (Command.contains("Teleport ")) {
			std::vector<std::string> args = TextManipUtils::SplitWhitespace(Command);
			FVector TeleportLoc = FVector();

			TeleportLoc.X = std::stoi(args[1]);
			TeleportLoc.Y = std::stoi(args[2]);
			TeleportLoc.Z = std::stoi(args[3]);

			PC->Pawn->K2_TeleportTo(TeleportLoc, PC->Pawn->K2_GetActorRotation());
			Log("Teleported: X: " + args[1] + " Y: " + args[2] + " Z: " + args[3]);
		}
		else if (Command == "StartEarlyBus") {
			if (GameState->GamePhase == EAthenaGamePhase::Warmup
				&& GameState->WarmupCountdownEndTime > UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 10.f) {

				auto TS = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
				auto DR = 10.f;

				GameState->WarmupCountdownEndTime = TS + DR;
				GameMode->WarmupCountdownDuration = DR;
				GameState->WarmupCountdownStartTime = TS;
				GameMode->WarmupEarlyCountdownDuration = DR;
			}
		}
		else if (Command == "SpawnGuards") {
			BotSpawner::SpawnGuards();
		}
	}

	static void ServerExecuteInventoryItem(AFortPlayerControllerAthena* PC, FGuid Guid)
	{
		if (!PC || !PC->MyFortPawn)
			return;

		FFortItemEntry* ItemEntry = Inventory::FindItemEntryByGuid(PC, Guid);
		if (!ItemEntry || !ItemEntry->ItemDefinition) {
			return;
		}

		if (ItemEntry->ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass())) {
			UFortGadgetItemDefinition* Gadget = (UFortGadgetItemDefinition*)ItemEntry->ItemDefinition;
			PC->MyFortPawn->EquipWeaponDefinition(Gadget->GetWeaponItemDefinition(), ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false);
			return;
		}

		PC->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemEntry->ItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false);
		return;
	}

	void ServerAttemptInventoryDrop(AFortPlayerControllerAthena* PC, FGuid ItemGuid, int Count, bool bTrash)
	{
		FFortItemEntry* Entry = Inventory::FindItemEntryByGuid(PC, ItemGuid);
		if (Entry) {
			AFortPlayerPawn* Pawn = (AFortPlayerPawn*)PC->Pawn;
			AFortPickup* Pickup = SpawnPickup(Entry->ItemDefinition, Count, Entry->LoadedAmmo, PC->Pawn->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
			Inventory::RemoveItem(PC, Entry->ItemDefinition, Count);
			Pickup->PawnWhoDroppedPickup = Pawn;
		}
	}

	void (*ServerCreateBuildingActorOG)(AFortPlayerControllerAthena* PC, FCreateBuildingActorData& CreateBuildingData);
	void ServerCreateBuildingActor(AFortPlayerControllerAthena* PC, FCreateBuildingActorData& CreateBuildingData) {
		//Log("ServerCreateBuildingActor Called!");
		if (!PC) {
			Log("No PC!");
			return;
		}

		UClass* BuildingClass = PC->BroadcastRemoteClientInfo->RemoteBuildableClass.Get();

		TArray<ABuildingSMActor*> BuildingsToRemove;
		char BuildRestrictionFlag;
		if (CantBuild(UWorld::GetWorld(), BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, CreateBuildingData.bMirrored, &BuildingsToRemove, &BuildRestrictionFlag))
		{
			Log("CantBuild!");
			return;
		}

		auto ResourceItemDefinition = UFortKismetLibrary::GetDefaultObj()->K2_GetResourceItemDefinition(((ABuildingSMActor*)BuildingClass->DefaultObject)->ResourceType);
		Inventory::RemoveItem(PC, ResourceItemDefinition, 10);

		ABuildingSMActor* PlacedBuilding = SpawnActor<ABuildingSMActor>(CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, PC, BuildingClass);
		PlacedBuilding->bPlayerPlaced = true;
		PlacedBuilding->InitializeKismetSpawnedBuildingActor(PlacedBuilding, PC, true, nullptr);
		PlacedBuilding->TeamIndex = ((AFortPlayerStateAthena*)PC->PlayerState)->TeamIndex;
		PlacedBuilding->Team = EFortTeam(PlacedBuilding->TeamIndex);

		for (size_t i = 0; i < BuildingsToRemove.Num(); i++)
		{
			BuildingsToRemove[i]->K2_DestroyActor();
		}
		BuildingsToRemove.Free();
	}

	void (*ServerBeginEditingBuildingActorOG)(AFortPlayerControllerAthena* PC, ABuildingSMActor* BuildingActorToEdit);
	void ServerBeginEditingBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* BuildingActorToEdit)
	{
		//Log("ServerBeginEditingBuildingActor Called!");
		if (!BuildingActorToEdit || !BuildingActorToEdit->bPlayerPlaced || !PC->MyFortPawn)
			return;

		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		BuildingActorToEdit->SetNetDormancy(ENetDormancy::DORM_Awake);
		BuildingActorToEdit->EditingPlayer = PlayerState;

		for (int i = 0; i < PC->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			auto Item = PC->WorldInventory->Inventory.ItemInstances[i];
			if (Item->GetItemDefinitionBP()->IsA(UFortEditToolItemDefinition::StaticClass()))
			{
				PC->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Item->GetItemDefinitionBP(), Item->GetItemGuid(), Item->GetTrackerGuid(), false);
				break;
			}
		}

		if (!PC->MyFortPawn->CurrentWeapon || !PC->MyFortPawn->CurrentWeapon->IsA(AFortWeap_EditingTool::StaticClass()))
			return;

		AFortWeap_EditingTool* EditTool = (AFortWeap_EditingTool*)PC->MyFortPawn->CurrentWeapon;
		EditTool->EditActor = BuildingActorToEdit;
		EditTool->OnRep_EditActor();

		return ServerBeginEditingBuildingActorOG(PC, BuildingActorToEdit);
	}

	void ServerEndEditingBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* BuildingActorToStopEditing) {
		if (!BuildingActorToStopEditing || !PC->MyFortPawn || BuildingActorToStopEditing->bDestroyed == 1 || BuildingActorToStopEditing->EditingPlayer != PC->PlayerState)
			return;
		BuildingActorToStopEditing->SetNetDormancy(ENetDormancy::DORM_DormantAll);
		BuildingActorToStopEditing->EditingPlayer = nullptr;
		for (size_t i = 0; i < PC->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			auto Item = PC->WorldInventory->Inventory.ItemInstances[i];
			if (Item->GetItemDefinitionBP()->IsA(UFortEditToolItemDefinition::StaticClass()))
			{
				PC->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Item->GetItemDefinitionBP(), Item->GetItemGuid(), Item->GetTrackerGuid(), false);
				break;
			}
		}
		if (!PC->MyFortPawn->CurrentWeapon || !PC->MyFortPawn->CurrentWeapon->WeaponData || !PC->MyFortPawn->CurrentWeapon->IsA(AFortWeap_EditingTool::StaticClass()))
			return;

		AFortWeap_EditingTool* EditTool = (AFortWeap_EditingTool*)PC->MyFortPawn->CurrentWeapon;
		EditTool->EditActor = nullptr;
		EditTool->OnRep_EditActor();
	}

	void ServerEditBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored) {
		if (!BuildingActorToEdit || BuildingActorToEdit->EditingPlayer != PC->PlayerState || !NewBuildingClass.Get() || BuildingActorToEdit->bDestroyed == 1)
			return;

		BuildingActorToEdit->SetNetDormancy(ENetDormancy::DORM_DormantAll);
		BuildingActorToEdit->EditingPlayer = nullptr;
		ABuildingSMActor* EditedBuildingActor = ReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass.Get(), 0, RotationIterations, bMirrored, PC);
		if (EditedBuildingActor)
			EditedBuildingActor->bPlayerPlaced = true;
	}

	void ServerRepairBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* BuildingActorToRepair) {
		auto FortKismet = (UFortKismetLibrary*)UFortKismetLibrary::StaticClass()->DefaultObject;
		if (!BuildingActorToRepair)
			return;

		if (BuildingActorToRepair->EditingPlayer)
		{
			return;
		}

		float BuildingHealthPercent = BuildingActorToRepair->GetHealthPercent();
		float BuildingCost = 10;
		float RepairCostMultiplier = 0.75;

		float BuildingHealthPercentLost = 1.0f - BuildingHealthPercent;
		float RepairCostUnrounded = (BuildingCost * BuildingHealthPercentLost) * RepairCostMultiplier;
		float RepairCost = std::floor(RepairCostUnrounded > 0 ? RepairCostUnrounded < 1 ? 1 : RepairCostUnrounded : 0);
		if (RepairCost < 0)
			return;

		auto ResourceDef = FortKismet->K2_GetResourceItemDefinition(BuildingActorToRepair->ResourceType);
		if (!ResourceDef)
			return;

		if (!PC->bBuildFree)
		{
			Inventory::RemoveItem(PC, ResourceDef, (int)RepairCost);
		}

		BuildingActorToRepair->RepairBuilding(PC, (int)RepairCost);
	}

	inline void (*ClientOnPawnDiedOG)(AFortPlayerControllerAthena* PC, FFortPlayerDeathReport DeathReport);
	inline void ClientOnPawnDied(AFortPlayerControllerAthena* PC, FFortPlayerDeathReport DeathReport) {
		auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		if (!PlayerState) {
			return;
		}
		Log(PlayerState->GetPlayerName().ToString() + " Died!");

		AFortPlayerPawnAthena* KillerPawn = (AFortPlayerPawnAthena*)DeathReport.KillerPawn;
		AFortPlayerStateAthena* KillerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;

		FVector DeathLocation = PC->Pawn->K2_GetActorLocation();
		PlayerState->PawnDeathLocation = DeathLocation;

		FDeathInfo& DeathInfo = PlayerState->DeathInfo;
		DeathInfo.bDBNO = false;
		DeathInfo.bInitialized = true;
		DeathInfo.DeathCause = PlayerState->ToDeathCause(DeathInfo.DeathTags, DeathInfo.bDBNO);
		DeathInfo.DeathLocation = PC->Pawn->K2_GetActorLocation();
		DeathInfo.DeathTags = DeathReport.Tags;
		DeathInfo.Distance = (KillerPawn ? KillerPawn->GetDistanceTo(PC->Pawn) : ((AFortPlayerPawnAthena*)PC->Pawn)->LastFallDistance);
		DeathInfo.Downer = KillerState;
		PlayerState->DeathInfo.FinisherOrDowner = DeathReport.KillerPlayerState ? DeathReport.KillerPlayerState : PC->PlayerState;

		// Dont think these tags are correct whatsoever but whatever
		DeathInfo.FinisherOrDownerTags = DeathReport.Tags;
		DeathInfo.VictimTags = DeathReport.Tags;
		PlayerState->OnRep_DeathInfo();
		RemoveFromAlivePlayers(GameMode, PC, PlayerState, KillerPawn, DeathReport.KillerWeapon, (uint8)PlayerState->DeathInfo.DeathCause, 0);
		PC->bMarkedAlive = false;

		if (!GameState->IsRespawningAllowed(PlayerState))
		{
			if (PC && PC->WorldInventory)
			{
				for (size_t i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
				{
					if (((UFortWorldItemDefinition*)PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition)->bCanBeDropped)
					{
						SpawnPickup(PC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.ItemDefinition, PC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.Count, PC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.LoadedAmmo, DeathLocation, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, PC->MyFortPawn);
					}
				}
			}

			FAthenaRewardResult Result;
			UFortPlayerControllerAthenaXPComponent* XPComponent = PC->XPComponent;
			Result.TotalBookXpGained = XPComponent->TotalXpEarned;
			Result.TotalSeasonXpGained = XPComponent->TotalXpEarned;

			PC->ClientSendEndBattleRoyaleMatchForPlayer(true, Result);

			FAthenaMatchStats Stats;
			FAthenaMatchTeamStats TeamStats;

			if (PlayerState)
			{
				PlayerState->Place = GameMode->AliveBots.Num() + GameMode->AlivePlayers.Num();
				PlayerState->OnRep_Place();
			}

			for (size_t i = 0; i < 20; i++)
			{
				Stats.Stats[i] = 0;
			}

			Stats.Stats[3] = PlayerState->KillScore;

			TeamStats.Place = PlayerState->Place;
			TeamStats.TotalPlayers = GameState->TotalPlayers;

			PC->ClientSendMatchStatsForPlayer(Stats);
			PC->ClientSendTeamStatsForPlayer(TeamStats);
		}

		return ClientOnPawnDiedOG(PC, DeathReport);
	}

	inline void (*ServerAttemptExitVehicleOG)(AFortPlayerController* PC);
	inline void ServerAttemptExitVehicle(AFortPlayerControllerZone* PC)
	{
		Log("ServerAttemptExitVehicle Called!");
		if (!PC) {
			Log("PC: " + PC->GetName());
			return ServerAttemptExitVehicleOG(PC);
		}

		auto Pawn = (AFortPlayerPawn*)PC->Pawn;

		ServerAttemptExitVehicleOG(PC);

		if (!Pawn->CurrentWeapon || !Pawn->CurrentWeapon->IsA(AFortWeaponRangedForVehicle::StaticClass()))
			return;

		Log(Pawn->CurrentWeapon->GetWeaponData()->GetName());
		Inventory::RemoveItem((AFortPlayerController*)Pawn->Controller, Pawn->CurrentWeapon->GetWeaponData(), 1);

		UFortWorldItemDefinition* ItemDef = ((AFortPlayerControllerAthena*)PC)->SwappingItemDefinition;
		if (!ItemDef)
			return;

		FFortItemEntry* ItemEntry = Inventory::FindItemEntryByDef(PC, ItemDef);
		if (!ItemEntry)
			return;

		Log(ItemEntry->ItemDefinition->GetName());
		PC->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemDef, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false);
	}

	inline void (*ServerRequestSeatChangeOG)(AFortPlayerController* PC, int32 TargetSeatIndex);
	inline void ServerRequestSeatChange(AFortPlayerControllerZone* PC, int32 TargetSeatIndex)
	{
		Log("ServerRequestSeatChange Called!");
		if (!PC)
			return;

		Log(std::to_string(TargetSeatIndex));
		auto Pawn = (AFortPlayerPawn*)PC->Pawn;

		ServerRequestSeatChangeOG(PC, TargetSeatIndex);

		if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->IsA(AFortWeaponRangedForVehicle::StaticClass())) {
			Inventory::RemoveItem((AFortPlayerController*)Pawn->Controller, Pawn->CurrentWeapon->GetWeaponData(), 1);
		}

		auto Vehicle = PC->MyFortPawn->BP_GetVehicle();
		if (Vehicle)
		{
			auto WeaponComp = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());
			if (WeaponComp)
			{
				auto SeatIdx = PC->MyFortPawn->GetVehicleSeatIndex();
				if (!WeaponComp->WeaponSeatDefinitions.IsValidIndex(SeatIdx)) {
					return;
				}
				Inventory::GiveItem(PC, WeaponComp->WeaponSeatDefinitions[SeatIdx].VehicleWeapon, 1, 9999);
				for (size_t i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
				{
					if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == WeaponComp->WeaponSeatDefinitions[SeatIdx].VehicleWeapon)
					{
						FFortItemEntry ItemEntry = PC->WorldInventory->Inventory.ReplicatedEntries[i];
						PC->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid, ItemEntry.TrackerGuid, false);
						break;
					}
				}
			}
		}
	}

	void Hook() {
		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x125, ServerAcknowledgePossession, (LPVOID*)&ServerAcknowledgePossessionOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x296, ServerReadyToStartMatch, (LPVOID*)&ServerReadyToStartMatchOG);

		HookVTable(UFortControllerComponent_Aircraft::GetDefaultObj(), 0x9F, ServerAttemptAircraftJump, nullptr);

		// TODO: Move vehicle specific stuff into its own header file and remove it from serverattemptinteract
		MH_CreateHook((LPVOID)(ImageBase + 0x68EA5A8), ServerAttemptInteract, (LPVOID*)&ServerAttemptInteractOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x1ED, ServerPlayEmoteItem, nullptr);
		MH_CreateHook((LPVOID)(ImageBase + 0x21F1BE4), MovingEmoteStopped, (LPVOID*)&MovingEmoteStoppedOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x292, ServerReturnToMainMenu, nullptr);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x1EB, ServerCheat, nullptr);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x231, ServerExecuteInventoryItem, nullptr);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x23F, ServerAttemptInventoryDrop, nullptr);

		HookVTable(AAthena_PlayerController_C::GetDefaultObj(), 0x254, ServerCreateBuildingActor, (LPVOID*)&ServerCreateBuildingActorOG);

		HookVTable(AAthena_PlayerController_C::GetDefaultObj(), 0x25B, ServerBeginEditingBuildingActor, (LPVOID*)&ServerBeginEditingBuildingActorOG);

		HookVTable(AAthena_PlayerController_C::GetDefaultObj(), 0x259, ServerEndEditingBuildingActor, nullptr);

		HookVTable(AAthena_PlayerController_C::GetDefaultObj(), 0x256, ServerEditBuildingActor, nullptr);

		HookVTable(AAthena_PlayerController_C::GetDefaultObj(), 0x250, ServerRepairBuildingActor, nullptr);

		MH_CreateHook((LPVOID)(ImageBase + 0x73B8A4C), ClientOnPawnDied, (LPVOID*)&ClientOnPawnDiedOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x471, ServerAttemptExitVehicle, (PVOID*)&ServerAttemptExitVehicleOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x45D, ServerRequestSeatChange, (PVOID*)&ServerRequestSeatChangeOG);

		Log("PC Hooked!");
	}
}