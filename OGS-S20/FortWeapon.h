#pragma once
#include "framework.h"
#include "Inventory.h"

namespace FortWeapon {
	void (*OnReloadOG)(AFortWeapon* a1, int RemoveCount);
	void OnReload(AFortWeapon* a1, int RemoveCount)
	{
		if (!a1) return OnReloadOG(a1, RemoveCount);

		AFortPlayerPawn* Pawn = (AFortPlayerPawn*)a1->GetOwner();
		if (!Pawn || !Pawn->Controller) return OnReloadOG(a1, RemoveCount);

		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Pawn->Controller;
		if (!PC) return OnReloadOG(a1, RemoveCount);
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)Pawn->PlayerState;
		if (!PlayerState || PlayerState->bIsABot) return OnReloadOG(a1, RemoveCount);

		FFortItemEntry* WeaponItemEntry = Inventory::FindItemEntryByGuid(PC, a1->ItemEntryGuid);
		if (!WeaponItemEntry || !WeaponItemEntry->ItemDefinition) return OnReloadOG(a1, RemoveCount);

		UFortWorldItemDefinition* AmmoItemDef = a1->WeaponData ? a1->WeaponData->GetAmmoWorldItemDefinition_BP() : nullptr;
		if (!AmmoItemDef) return OnReloadOG(a1, RemoveCount);

		FFortItemEntry* AmmoItemEntry = Inventory::FindItemEntryByDef(PC, AmmoItemDef);
		if (AmmoItemEntry)
		{
			Inventory::RemoveItem(PC, AmmoItemEntry->ItemDefinition, RemoveCount);
		}
		else
		{
			int MaxStackSize = WeaponItemEntry->ItemDefinition->MaxStackSize.Value;
			if (MaxStackSize > 1) Inventory::RemoveItem(PC, WeaponItemEntry->ItemDefinition, RemoveCount);
		}

		WeaponItemEntry->LoadedAmmo = a1->AmmoCount;

		Inventory::Update(PC, WeaponItemEntry);

		return OnReloadOG(a1, RemoveCount);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x7115D20), OnReload, (LPVOID*)&OnReloadOG);

		Log("FortWeapon Hooked!");
	}
}
