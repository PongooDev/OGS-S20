#pragma once
#include "framework.h"
#include "Inventory.h"
#include "Quests.h"

namespace BuildingActor {
	void (*OnDamageServerOG)(ABuildingActor* This, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext);
	void OnDamageServer(ABuildingActor* This, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext) {
		if (!This || !InstigatedBy || !InstigatedBy->IsA(AFortPlayerControllerAthena::StaticClass()) || !DamageCauser->IsA(AFortWeapon::StaticClass()) || !((AFortWeapon*)DamageCauser)->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) || This->bPlayerPlaced) {
			return OnDamageServerOG(This, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		}

		ABuildingSMActor* BuildingSMActor = (ABuildingSMActor*)This;
		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)InstigatedBy;
		if (!PC->Pawn) {
			return OnDamageServerOG(This, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		}
		AFortPlayerPawnAthena* Pawn = (AFortPlayerPawnAthena*)PC->Pawn;

		int MaterialCount = (Damage / (UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(4, 8)));

		PC->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->ResourceType, MaterialCount, BuildingSMActor->bDestroyed, (Damage == 100.f));

		UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType);
		if (!ResourceItemDefinition) {
			return OnDamageServerOG(This, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		}

		AFortPickup* Pickup = SpawnPickup(ResourceItemDefinition, MaterialCount, 0, PC->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, true, PC->MyFortPawn);
		Pawn->ServerHandlePickup(Pickup, 0.3f, FVector(), false);

		return OnDamageServerOG(This, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x714BD34), OnDamageServer, (LPVOID*)&OnDamageServerOG);

		Log("Hooked Building!");
	}
}