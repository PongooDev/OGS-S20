#pragma once
#include "framework.h"
#include "Inventory.h"

namespace Pawn {
	void ServerHandlePickup(AFortPlayerPawnAthena* Pawn, AFortPickup* PickUp, float InFlyTime, FVector& InStartDirection, bool bPlayPickupSound) {
		//Log("ServerHandlePickup Called!");
		if (!Pawn || !PickUp) {
			return;
		}
		if (PickUp->bPickedUp) {
			return;
		}

		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Pawn->Controller;

		FFortItemEntry& PickupItemEntry = PickUp->PrimaryPickupItemEntry;

		PickUp->PickupLocationData.PickupGuid = PickUp->PrimaryPickupItemEntry.ItemGuid;
		PickUp->PickupLocationData.PickupTarget = Pawn;
		PickUp->PickupLocationData.ItemOwner = Pawn;
		PickUp->PickupLocationData.FlyTime = 0.3f;
		PickUp->PickupLocationData.bPlayPickupSound = true;
		PickUp->OnRep_PickupLocationData();

		PickUp->bPickedUp = true;
		PickUp->OnRep_bPickedUp();
	}

	__int64 (*CompletePickupAnimationOG)(AFortPickup* Pickup);
	__int64 CompletePickupAnimation(AFortPickup* Pickup)
	{
		//Log("CompletePickupAnimation Called!");
		if (!Pickup) {
			Log("No Pickup!");
			return CompletePickupAnimationOG(Pickup);
		}

		FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
		FFortItemEntry& PickupEntry = Pickup->PrimaryPickupItemEntry;

		AFortPlayerPawnAthena* Pawn = (AFortPlayerPawnAthena*)PickupLocationData.PickupTarget;
		if (!Pawn) return CompletePickupAnimationOG(Pickup);

		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Pawn->Controller;
		if (!PC) return CompletePickupAnimationOG(Pickup);
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		if (!PlayerState) return CompletePickupAnimationOG(Pickup);
		if (PlayerState->bIsABot) return CompletePickupAnimationOG(Pickup);

		UFortItemDefinition* PickupItemDefinition = PickupEntry.ItemDefinition;

		int PickupCount = PickupEntry.Count;
		int PickupLoadedAmmo = PickupEntry.LoadedAmmo;
		int PickupMaxStackSize = Inventory::GetMaxStack(PickupItemDefinition);
		if (!PC->WorldInventory) return CompletePickupAnimationOG(Pickup);
		FFortItemEntry* ItemEntry = Inventory::FindItemEntryByDef(PC, PickupItemDefinition);

		AFortWeapon* CurrentWeapon = Pawn->CurrentWeapon;
		if (!CurrentWeapon || !CurrentWeapon->WeaponData) {
			return CompletePickupAnimationOG(Pickup);
		}

		FVector Drop = Pawn->K2_GetActorLocation() + Pawn->GetActorForwardVector() * 100.f;
		if (ItemEntry) {
			if (PickupItemDefinition->IsStackable()) {
				bool bCanPickup = (ItemEntry->Count + PickupCount) <= PickupMaxStackSize;
				//Log(std::to_string((ItemEntry->Count + PickupCount)));
				//Log("PickupMaxStackSize: " + std::to_string(PickupMaxStackSize));
				if (PickupItemDefinition->IsA(UFortTrapItemDefinition::StaticClass())) {
					bCanPickup = true;
				}
				if (bCanPickup) {
					Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo, true);
				}
				else {
					int Space = PickupMaxStackSize - ItemEntry->Count;
					int AddToStack = UKismetMathLibrary::GetDefaultObj()->Min(Space, PickupCount);
					int LeftOver = PickupCount - AddToStack;

					if (AddToStack > 0) {
						Inventory::GiveItem(PC, PickupItemDefinition, AddToStack, 0, true);
						SpawnPickup(PickupItemDefinition, LeftOver, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
					}
					else {
						if (Inventory::GetQuickBars(CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
							FFortItemEntry* CurrentWeaponItemEntry = Inventory::FindItemEntryByDef(PC, CurrentWeapon->WeaponData);

							SpawnPickup(CurrentWeapon->WeaponData, CurrentWeaponItemEntry->Count, CurrentWeaponItemEntry->LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
							Inventory::RemoveItem(PC, CurrentWeapon->WeaponData);
							Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0, false);
						}
						else {
							SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
						}
					}
				}
			}
			else {
				if (Inventory::GetQuickBars(CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
					FFortItemEntry* CurrentWeaponItemEntry = Inventory::FindItemEntryByDef(PC, CurrentWeapon->WeaponData);

					SpawnPickup(CurrentWeapon->WeaponData, CurrentWeaponItemEntry->Count, CurrentWeaponItemEntry->LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
					Inventory::RemoveItem(PC, CurrentWeapon->WeaponData);
					Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0, false);
				}
				else {
					SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
				}
			}
		}
		else {
			if (PickupItemDefinition->IsStackable()) {
				if (PickupItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || PickupItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) ||
					PickupItemDefinition->IsA(UFortTrapItemDefinition::StaticClass())) {
					Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0);
				}
				else {
					if (Inventory::GetQuickBars(CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
						if (Inventory::IsInventoryFull(PC)) {
							FFortItemEntry* CurrentWeaponItemEntry = Inventory::FindItemEntryByDef(PC, CurrentWeapon->WeaponData);

							SpawnPickup(CurrentWeapon->WeaponData, CurrentWeaponItemEntry->Count, CurrentWeaponItemEntry->LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
							Inventory::RemoveItem(PC, CurrentWeapon->WeaponData);
							Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0);
						}
						else {
							Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0);
						}
					}
					else {
						if (Inventory::IsInventoryFull(PC)) {
							SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
						}
						else {
							Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, 0);
						}
					}
				}
			}
			else {
				if (PickupItemDefinition->IsA(UFortTrapItemDefinition::StaticClass())) {
					Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
				}
				else if (Inventory::IsInventoryFull(PC)) {
					if (Inventory::GetQuickBars(CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
						FFortItemEntry* CurrentWeaponItemEntry = Inventory::FindItemEntryByDef(PC, CurrentWeapon->WeaponData);

						SpawnPickup(CurrentWeapon->WeaponData, CurrentWeaponItemEntry->Count, CurrentWeaponItemEntry->LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
						Inventory::RemoveItem(PC, CurrentWeapon->WeaponData);
						Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
					}
					else {
						SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);
					}
				}
				else {
					Inventory::GiveItem(PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
				}
			}
		}

		Inventory::Update(PC);

		Pickup->K2_DestroyActor();
		return CompletePickupAnimationOG(Pickup);
	}

	void (*NetMulticast_Athena_BatchedDamageCuesOG)(AFortPlayerPawnAthena* Pawn, FAthenaBatchedDamageGameplayCues_Shared& SharedData, FAthenaBatchedDamageGameplayCues_NonShared& NonSharedData);
	void NetMulticast_Athena_BatchedDamageCues(AFortPlayerPawnAthena* Pawn, FAthenaBatchedDamageGameplayCues_Shared& SharedData, FAthenaBatchedDamageGameplayCues_NonShared& NonSharedData)
	{
		if (!Pawn || Pawn->PlayerState->bIsABot)
			return;

		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Pawn->Controller;
		if (!PC) return;

		if (Pawn->CurrentWeapon) {
			FFortItemEntry* ItemEntry = Inventory::FindItemEntryByGuid(PC, Pawn->CurrentWeapon->ItemEntryGuid);
			if (!ItemEntry) return;
			ItemEntry->LoadedAmmo = Pawn->CurrentWeapon->AmmoCount;
			Inventory::Update(PC, ItemEntry);
		}

		return;
	}

	void (*OnCapsuleBeginOverlapOG)(AFortPlayerPawn* Pawn, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult);
	void OnCapsuleBeginOverlap(AFortPlayerPawn* Pawn, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult)
	{
		if (!OtherActor) {
			return;
		}

		if (OtherActor->IsA(AFortPickup::StaticClass()))
		{
			auto PC = (AFortPlayerControllerAthena*)Pawn->GetOwner();
			if (PC->PlayerState->bIsABot) return OnCapsuleBeginOverlapOG(Pawn, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

			AFortPickup* Pickup = (AFortPickup*)OtherActor;

			if (Pickup->PawnWhoDroppedPickup == Pawn)
				return OnCapsuleBeginOverlapOG(Pawn, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

			UFortWorldItemDefinition* Def = (UFortWorldItemDefinition*)Pickup->PrimaryPickupItemEntry.ItemDefinition;

			if (!Def) {
				return OnCapsuleBeginOverlapOG(Pawn, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
			}

			FFortItemEntry* ItemEntry = Inventory::FindItemEntryByDef(PC, Def);
			auto Count = ItemEntry ? ItemEntry->Count : 0;

			if (Def->IsStackable()) {
				if (Def->IsA(UFortAmmoItemDefinition::StaticClass()) || Def->IsA(UFortResourceItemDefinition::StaticClass()) || Def->IsA(UFortTrapItemDefinition::StaticClass())) {
					if (Count < Inventory::GetMaxStack(Def)) {
						Pawn->ServerHandlePickup(Pickup, 0.30f, FVector(), true);
					}
				}
				else if (ItemEntry) {
					if (Count < Inventory::GetMaxStack(Def)) {
						Pawn->ServerHandlePickup(Pickup, 0.30f, FVector(), true);
					}
				}
			}
		}

		return OnCapsuleBeginOverlapOG(Pawn, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}

	void Hook() {
		HookVTable(APlayerPawn_Athena_C::GetDefaultObj(), 0x22B, ServerHandlePickup, nullptr);

		MH_CreateHook((LPVOID)(ImageBase + 0x234E47C), CompletePickupAnimation, (LPVOID*)&CompletePickupAnimationOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x10A2114), NetMulticast_Athena_BatchedDamageCues, (LPVOID*)&NetMulticast_Athena_BatchedDamageCuesOG);

		Log("Pawn Hooked!");
	}
}