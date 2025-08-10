#pragma once
#include "framework.h"

namespace Inventory {
	bool CompareGuids(FGuid Guid1, FGuid Guid2) {
		if (Guid1.A == Guid2.A
			&& Guid1.B == Guid2.B
			&& Guid1.C == Guid2.C
			&& Guid1.D == Guid2.D) {
			return true;
		}
		else {
			return false;
		}
	}

	void Update(AFortPlayerController* PC, FFortItemEntry* ItemEntry = nullptr)
	{
		PC->HandleWorldInventoryLocalUpdate();
		PC->WorldInventory->HandleInventoryLocalUpdate();
		PC->WorldInventory->bRequiresLocalUpdate = true;
		PC->WorldInventory->ForceNetUpdate();
		if (ItemEntry == nullptr)
			PC->WorldInventory->Inventory.MarkArrayDirty();
		else
			PC->WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
	}

	void GiveItem(AFortPlayerController* PC, UFortItemDefinition* Def, int Count, int LoadedAmmo, bool bShouldAddToExistingStack = false)
	{
		if (bShouldAddToExistingStack) {
			for (FFortItemEntry& ItemEntry : PC->WorldInventory->Inventory.ReplicatedEntries) {
				if (Def == ItemEntry.ItemDefinition) {
					ItemEntry.Count += Count;
					PC->WorldInventory->Inventory.MarkItemDirty(ItemEntry);
					Update(PC);
					break;
				}
			}
			return;
		}
		UFortWorldItem* Item = Cast<UFortWorldItem>(Def->CreateTemporaryItemInstanceBP(Count, 0));
		Item->SetOwningControllerForTemporaryItem(PC);
		Item->OwnerInventory = PC->WorldInventory;
		Item->ItemEntry.LoadedAmmo = LoadedAmmo;

		PC->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
		PC->WorldInventory->Inventory.ItemInstances.Add(Item);
		PC->WorldInventory->Inventory.MarkItemDirty(Item->ItemEntry);
		PC->WorldInventory->HandleInventoryLocalUpdate();
	}

	FFortItemEntry* GiveStack(AFortPlayerControllerAthena* PC, UFortItemDefinition* Def, int Count = 1, bool GiveLoadedAmmo = false, int LoadedAmmo = 0, bool Toast = false)
	{
		UFortWorldItem* Item = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, 0);

		Item->SetOwningControllerForTemporaryItem(PC);
		Item->OwnerInventory = PC->WorldInventory;
		Item->ItemEntry.ItemDefinition = Def;
		Item->ItemEntry.Count = Count;


		if (GiveLoadedAmmo)
		{
			Item->ItemEntry.LoadedAmmo = LoadedAmmo;
		}
		Item->ItemEntry.ReplicationKey++;

		PC->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
		PC->WorldInventory->Inventory.ItemInstances.Add(Item);

		Update(PC, nullptr);
		return &Item->ItemEntry;
	}

	void RemoveItem(AFortPlayerController* PC, UFortItemDefinition* Def, int Count = INT_MAX) {
		for (int i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			FFortItemEntry& ItemEntry = PC->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Def == ItemEntry.ItemDefinition) {
				ItemEntry.Count -= Count;
				if (ItemEntry.Count <= 0) {
					PC->WorldInventory->Inventory.ReplicatedEntries[i].StateValues.Free();
					PC->WorldInventory->Inventory.ReplicatedEntries.Remove(i);

					for (int i = 0; i < PC->WorldInventory->Inventory.ItemInstances.Num(); i++)
					{
						UFortWorldItem* WorldItem = PC->WorldInventory->Inventory.ItemInstances[i];
						if (WorldItem->ItemEntry.ItemDefinition == Def) {
							PC->WorldInventory->Inventory.ItemInstances.Remove(i);
						}
					}
				}
				else {
					Update(PC, &ItemEntry);
				}
				break;
			}
		}
		Update(PC);
	}

	FFortItemEntry* FindItemEntryByGuid(AFortPlayerController* PC, FGuid Guid)
	{
		for (auto& Entry : PC->WorldInventory->Inventory.ReplicatedEntries)
		{
			if (CompareGuids(Entry.ItemGuid, Guid))
			{
				return &Entry;
			}
		}
		return nullptr;
	}

	FFortItemEntry* FindItemEntryByDef(AFortPlayerController* PC, UFortItemDefinition* ItemDef)
	{
		if (!PC || !PC->WorldInventory || !ItemDef)
			return nullptr;
		for (int i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDef)
			{
				return &PC->WorldInventory->Inventory.ReplicatedEntries[i];
			}
		}
		return nullptr;
	}

	UFortWorldItem* FindItemInstanceByDef(AFortInventory* inv, UFortItemDefinition* ItemDefinition)
	{
		auto& ItemInstances = inv->Inventory.ItemInstances;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances[i];

			if (ItemInstance->ItemEntry.ItemDefinition == ItemDefinition)
				return ItemInstance;
		}

		return nullptr;
	}

	UFortWorldItem* FindItemInstanceByGuid(AFortInventory* inv, const FGuid& Guid)
	{
		auto& ItemInstances = inv->Inventory.ItemInstances;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances[i];

			if (CompareGuids(ItemInstance->ItemEntry.ItemGuid, Guid))
				return ItemInstance;
		}

		return nullptr;
	}

	bool IsPrimaryQuickbar(UFortItemDefinition* Def)
	{
		return
			Def->IsA(UFortConsumableItemDefinition::StaticClass()) ||
			Def->IsA(UFortWeaponRangedItemDefinition::StaticClass()) ||
			Def->IsA(UFortGadgetItemDefinition::StaticClass());
	}

	EFortQuickBars GetQuickBars(UFortItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
			!ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
			return EFortQuickBars::Primary;

		return EFortQuickBars::Secondary;
	}

	bool IsInventoryFull(AFortPlayerController* PC)
	{
		int ItemNumber = 0;
		auto InstancesPtr = &PC->WorldInventory->Inventory.ItemInstances;
		for (int i = 0; i < InstancesPtr->Num(); i++)
		{
			if (InstancesPtr->operator[](i))
			{
				if (GetQuickBars(InstancesPtr->operator[](i)->ItemEntry.ItemDefinition) == EFortQuickBars::Primary)
				{
					ItemNumber++;

					if (ItemNumber >= 5)
					{
						break;
					}
				}
			}
		}

		return ItemNumber >= 5;
	}

	float GetMaxStack(UFortItemDefinition* Def)
	{
		if (!Def->MaxStackSize.Curve.CurveTable)
			return Def->MaxStackSize.Value;
		EEvaluateCurveTableResult Result;
		float Ret;
		((UDataTableFunctionLibrary*)UDataTableFunctionLibrary::StaticClass()->DefaultObject)->EvaluateCurveTableRow(Def->MaxStackSize.Curve.CurveTable, Def->MaxStackSize.Curve.RowName, 0, &Result, &Ret, FString());
		return Ret;
	}
}