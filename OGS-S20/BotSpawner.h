#pragma once
#include "framework.h"
#include "FortAthenaAIBotController.h"
#include "BossesSpawnLocs.h"
#include "GuardSpawnLocs.h"
#include "NpcSpawnLocs.h"

namespace BotSpawner {
	TArray<FVector> GetSpawnLocations(std::string Name)
	{
		TArray<FVector> SpawnLocations;

		TArray<AFortAthenaPatrolPath*> PossibleSpawnPaths;
		for (auto& Path : GetAllActorsOfClass<AFortAthenaPatrolPathPointProvider>())
		{
			if (Path->FiltersTags.GameplayTags.Num() == 0)
				continue;
			auto PathName = Path->FiltersTags.GameplayTags[0].TagName.ToString();
			if (PathName.substr(PathName.rfind(L'.') + 1) == Name)
				PossibleSpawnPaths.Add(Path->AssociatedPatrolPath);
		}
		for (auto& PatrolPath : GetAllActorsOfClass<AFortAthenaPatrolPath>())
		{
			if (PatrolPath->GameplayTags.GameplayTags.Num() == 0)
				continue;
			auto PathName = PatrolPath->GameplayTags.GameplayTags[0].TagName.ToString();
			if (PathName.substr(PathName.rfind(L'.') + 1) == Name)
				PossibleSpawnPaths.Add(PatrolPath);
		}

		if (PossibleSpawnPaths.Num() > 0)
		{
			for (int i = 0; i < PossibleSpawnPaths.Num(); i++) {
				SpawnLocations.Add(PossibleSpawnPaths[i]->K2_GetActorLocation());
			}
		}
		else
		{
			Log("No Paths Found For Name: " + Name);
		}

		return SpawnLocations;
	}

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
		int AmountNpcsSpawned = 0;
		TArray<FVector> Locs;

		FortAthenaAIBotController::BotSpawnData BotSpawnData;

		FTransform Transform{};
		Transform.Translation = FVector();
		Transform.Rotation = FQuat();
		Transform.Scale3D = FVector{ 1,1,1 };

		Locs = GetSpawnLocations("TheOrigin");
		if (Locs.Num() > 0) {
			auto TheOrigin_SpawnerData = StaticLoadObject<UClass>("/NPCLibraryS20/NPCs/TheOrigin/BP_AIBotSpawnerData_NPC_S20_TheOrigin.BP_AIBotSpawnerData_NPC_S20_TheOrigin_C");
			auto TheOrigin_List = ((UFortAthenaAIBotSpawnerData*)TheOrigin_SpawnerData)->CreateComponentListFromClass(TheOrigin_SpawnerData, UWorld::GetWorld());

			Transform.Translation = Locs[rand() % (Locs.Num())];
			Log("X: " + std::to_string(Transform.Translation.X));
			Log("Y: " + std::to_string(Transform.Translation.Y));
			Log("Z: " + std::to_string(Transform.Translation.Z));

			int32 RequestID = ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(TheOrigin_List, Transform);
			BotSpawnData.RequestID = RequestID;
			BotSpawnData.BotSpawnerData = TheOrigin_SpawnerData;
			FortAthenaAIBotController::SpawnedBots.push_back(BotSpawnData);
			AmountNpcsSpawned++;
		}

		Log("Spawned " + std::to_string(AmountNpcsSpawned) + " Npcs!");
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

		static UClass* PhoebeSpawnerData;
		static UFortAthenaAISpawnerDataComponentList* ComponentList;
		if (!PhoebeSpawnerData) {
			PhoebeSpawnerData = StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");
			ComponentList = UFortAthenaAIBotSpawnerData::CreateComponentListFromClass(PhoebeSpawnerData, UWorld::GetWorld());
		}

		int32 RequestID = ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, Transform);
		BotSpawnData.RequestID = RequestID;
		BotSpawnData.BotSpawnerData = PhoebeSpawnerData;
		BotSpawnData.BotIDSuffix = L"Phoebe";
		FortAthenaAIBotController::SpawnedBots.push_back(BotSpawnData);
	}
}