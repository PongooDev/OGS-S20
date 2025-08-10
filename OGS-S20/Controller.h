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
					int Count = PC->WorldInventory->Inventory.ReplicatedEntries[i].Count;
					Inventory::RemoveItem(PC, PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition, Count);
				}
			}
		}

		PC->MyFortPawn->OnRep_IsInsideSafeZone();

		GameState->OnRep_SafeZoneDamage();
		GameState->OnRep_SafeZoneIndicator();
		GameState->OnRep_SafeZonePhase();
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
			SpawnPickup(Entry->ItemDefinition, Count, Entry->LoadedAmmo, PC->Pawn->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
			Inventory::RemoveItem(PC, Entry->ItemDefinition, Count);
		}
	}

	void Hook() {
		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x125, ServerAcknowledgePossession, (LPVOID*)&ServerAcknowledgePossessionOG);

		HookVTable(UFortControllerComponent_Aircraft::GetDefaultObj(), 0x9F, ServerAttemptAircraftJump, nullptr);

		// TODO: Move vehicle specific stuff into its own header file and remove it from serverattemptinteract
		MH_CreateHook((LPVOID)(ImageBase + 0x68EA5A8), ServerAttemptInteract, (LPVOID*)&ServerAttemptInteractOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x1ED, ServerPlayEmoteItem, nullptr);
		MH_CreateHook((LPVOID)(ImageBase + 0x21F1BE4), MovingEmoteStopped, (LPVOID*)&MovingEmoteStoppedOG);

		//HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), , ServerReturnToMainMenu, nullptr); (we gotta find this)

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x1EB, ServerCheat, nullptr);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x231, ServerExecuteInventoryItem, nullptr);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x23F, ServerAttemptInventoryDrop, nullptr);

		Log("PC Hooked!");
	}
}