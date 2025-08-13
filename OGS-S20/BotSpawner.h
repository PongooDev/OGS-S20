#pragma once
#include "framework.h"
#include "FortAthenaAIBotController.h"
#include "BossesSpawnLocs.h"
#include "GuardSpawnLocs.h"
#include "NpcSpawnLocs.h"

namespace BotSpawner {
	void SpawnBosses() {

	}

	void SpawnGuards() {
		int AmountGuardsSpawned = 0;
		FortAthenaAIBotController::BotSpawnData BotSpawnData;

		auto IO_Compound_SpawnerData = StaticLoadObject<UClass>("/IO_Guard/AI/NPCs/IO_Compound/BP_AIBotSpawnerData_IO_Compound.BP_AIBotSpawnerData_IO_Compound_C");
		auto IO_Compound_List = ((UFortAthenaAIBotSpawnerData*)IO_Compound_SpawnerData)->CreateComponentListFromClass(IO_Compound_SpawnerData, UWorld::GetWorld());

		for (auto Loc : GuardSpawnLocs::BlimpSpawnLocs) {
			FVector ChosenStartSpot = Loc[rand() % (Loc.size())];
			FTransform Transform{};
			Transform.Translation = ChosenStartSpot;
			Transform.Rotation = FQuat();
			Transform.Scale3D = FVector{ 1,1,1 };

			int32 RequestID = ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(IO_Compound_List, Transform);
			BotSpawnData.RequestID = RequestID;
			BotSpawnData.BotSpawnerData = IO_Compound_SpawnerData;
			FortAthenaAIBotController::SpawnedBots.push_back(BotSpawnData);
			AmountGuardsSpawned++;
		}

		Log("Spawned " + std::to_string(AmountGuardsSpawned) + " Guards!");
	}

	void SpawnNpcs() {

	}

	void SpawnPlayerBot() {
		if (PlayerStarts.Num() == 0) {
			Log("No PlayerStarts!");
			UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPlayerStartWarmup::StaticClass(), &PlayerStarts);
			return;
		}

		auto start = PlayerStarts[UKismetMathLibrary::RandomIntegerInRange(0, PlayerStarts.Num() - 1)];
		if (!start) {
			Log("No playerstart!");
			return;
		}
		FortAthenaAIBotController::BotSpawnData BotSpawnData;

		FTransform Transform{};
		Transform.Translation = start->K2_GetActorLocation();
		Transform.Rotation = FQuat();
		Transform.Scale3D = FVector{ 1,1,1 };

		static auto PhoebeSpawnerData = StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");
		auto ComponentList = UFortAthenaAIBotSpawnerData::CreateComponentListFromClass(PhoebeSpawnerData, UWorld::GetWorld());

		int32 RequestID = ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, Transform);
		BotSpawnData.RequestID = RequestID;
		BotSpawnData.BotSpawnerData = PhoebeSpawnerData;
		BotSpawnData.BotIDSuffix = L"Phoebe";
		FortAthenaAIBotController::SpawnedBots.push_back(BotSpawnData);
	}
}