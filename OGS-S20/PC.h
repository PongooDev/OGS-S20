#pragma once
#include "framework.h"
#include "Inventory.h"

namespace PC {
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
					Inventory::RemoveItem(PC, PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid, Count);
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

	// Yeah ill move this shit at some point, the gs is not so clean atm so at some point ill do a big cleanup
	AFortAthenaVehicle* (*ServerOnExitVehicleOG)(AFortPlayerPawn* PlayerPawn, ETryExitVehicleBehavior ExitForceBehavior, const bool bDestroyVehicleWhenForced);
	AFortAthenaVehicle* ServerOnExitVehicle(AFortPlayerPawn* Pawn, ETryExitVehicleBehavior ExitForceBehavior, const bool bDestroyVehicleWhenForced)
	{
		AFortAthenaVehicle* Vehicle = Pawn->BP_GetVehicle();

		if (!Vehicle)
			return ServerOnExitVehicleOG(Pawn, ExitForceBehavior, bDestroyVehicleWhenForced);

		UFortVehicleSeatWeaponComponent* SeatWeaponComponent = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());

		int32 VehicleSeatIndex = Pawn->GetVehicleSeatIndex();

		if (!SeatWeaponComponent || !SeatWeaponComponent->WeaponSeatDefinitions.IsValidIndex(VehicleSeatIndex))
			return ServerOnExitVehicleOG(Pawn, ExitForceBehavior, bDestroyVehicleWhenForced);

		FWeaponSeatDefinition* WeaponSeatDefinition = &SeatWeaponComponent->WeaponSeatDefinitions[VehicleSeatIndex];

		if (!WeaponSeatDefinition)
			return ServerOnExitVehicleOG(Pawn, ExitForceBehavior, bDestroyVehicleWhenForced);

		UnEquipVehicleWeapon(SeatWeaponComponent, Pawn, WeaponSeatDefinition, false);
		AFortPlayerController* PC = (AFortPlayerController*)Pawn->Controller;
		if (!PC)
			return ServerOnExitVehicleOG(Pawn, ExitForceBehavior, bDestroyVehicleWhenForced);

		// idfk why this dont work
		for (auto Item : PC->WorldInventory->Inventory.ReplicatedEntries) {
			if (Item.ItemDefinition->IsA(UAthenaPickaxeItemDefinition::StaticClass()))
			{
				PC->ServerExecuteInventoryItem(Item.ItemGuid);
				break;
			}
		}

		return ServerOnExitVehicleOG(Pawn, ExitForceBehavior, bDestroyVehicleWhenForced);
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

	void Hook() {
		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x125, ServerAcknowledgePossession, (LPVOID*)&ServerAcknowledgePossessionOG);

		HookVTable(UFortControllerComponent_Aircraft::GetDefaultObj(), 0x9F, ServerAttemptAircraftJump, nullptr);

		// TODO: Move vehicle specific stuff into its own header file and remove it from serverattemptinteract
		MH_CreateHook((LPVOID)(ImageBase + 0x68EA5A8), ServerAttemptInteract, (LPVOID*)&ServerAttemptInteractOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x6E6C820), ServerOnExitVehicle, (LPVOID*)&ServerOnExitVehicleOG);

		HookVTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x1ED, ServerPlayEmoteItem, nullptr);
		MH_CreateHook((LPVOID)(ImageBase + 0x21F1BE4), MovingEmoteStopped, (LPVOID*)&MovingEmoteStoppedOG);

		Log("PC Hooked!");
	}
}