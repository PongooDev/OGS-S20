#pragma once
#include "framework.h"
#include "Globals.h"
#include "Replication.h"

namespace Tick {
	void (*ServerReplicateActors)(void*) = decltype(ServerReplicateActors)(UReplicationGraph::GetDefaultObj()->VTable[0x66]);

	inline void (*TickFlushOG)(UNetDriver*, float);
	void TickFlush(UNetDriver* Driver, float DeltaTime)
	{
		if (!Driver)
			return TickFlushOG(Driver, DeltaTime);

		AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		Replication::ServerReplicateActors(Driver, DeltaTime);
		//ServerReplicateActors(Driver->ReplicationDriver);

		if (Driver->ClientConnections.Num() != 0) {
			if (GameState->GamePhase == EAthenaGamePhase::Warmup
				&& (GameMode->NumPlayers + GameMode->NumBots) >= Globals::MinPlayersForEarlyStart
				&& GameState->WarmupCountdownEndTime > UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 10.f) {

				auto TS = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
				auto DR = 10.f;

				GameState->WarmupCountdownEndTime = TS + DR;
				GameMode->WarmupCountdownDuration = DR;
				GameState->WarmupCountdownStartTime = TS;
				GameMode->WarmupEarlyCountdownDuration = DR;
			}
		}

		return TickFlushOG(Driver, DeltaTime);
	}


	inline float GetMaxTickRate()
	{
		return Globals::MaxTickRate;
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0xe1b0cc), GetMaxTickRate, nullptr);
		MH_CreateHook((LPVOID)(ImageBase + 0xd56288), TickFlush, (LPVOID*)&TickFlushOG);

		Log("Tick Hooked!");
	}
}