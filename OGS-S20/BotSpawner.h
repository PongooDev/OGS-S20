#pragma once
#include "framework.h"
#include "BossesSpawnLocs.h"
#include "GuardSpawnLocs.h"
#include "NpcSpawnLocs.h"

namespace BotSpawner {
	void SpawnBosses() {

	}

	void SpawnGuards() {
		int AmountGuardsSpawned = 0;

		auto IO_Compound_SpawnerData = StaticLoadObject<UClass>("/IO_Guard/AI/NPCs/IO_Compound/BP_AIBotSpawnerData_IO_Compound.BP_AIBotSpawnerData_IO_Compound_C");
		auto IO_Compound_List = ((UFortAthenaAIBotSpawnerData*)IO_Compound_SpawnerData)->CreateComponentListFromClass(IO_Compound_SpawnerData, UWorld::GetWorld());

		for (auto Loc : GuardSpawnLocs::BlimpSpawnLocs) {
			FVector ChosenStartSpot = Loc[rand() % (Loc.size())];
			FTransform Transform{};
			Transform.Translation = ChosenStartSpot;
			Transform.Rotation = FQuat();
			Transform.Scale3D = FVector{ 1,1,1 };

			((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(IO_Compound_List, Transform);
			AmountGuardsSpawned++;
		}

		Log("Spawned " + std::to_string(AmountGuardsSpawned) + " Guards!");
	}

	void SpawnNpcs() {

	}
}