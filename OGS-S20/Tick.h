#pragma once
#include "framework.h"
#include "Globals.h"
#include "Replication.h"
#include "BotSpawner.h";
#include "FortAthenaAIBotController.h"

namespace Tick {
	void (*ServerReplicateActors)(void*) = decltype(ServerReplicateActors)(UReplicationGraph::GetDefaultObj()->VTable[0x66]);

	EAthenaGamePhaseStep GetCurrentGamePhaseStep(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState) {
		float CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());

		if (GameState->GamePhase == EAthenaGamePhase::Setup) {
			return EAthenaGamePhaseStep::Setup;
		}
		else if (GameState->GamePhase == EAthenaGamePhase::Warmup) {
			if (GameState->WarmupCountdownEndTime > CurrentTime + 10.f) {
				return EAthenaGamePhaseStep::Warmup;
			}
			else {
				return EAthenaGamePhaseStep::GetReady;
			}
		}
		else if (GameState->GamePhase == EAthenaGamePhase::Aircraft) {
			if (GameState->GamePhaseStep > EAthenaGamePhaseStep::BusLocked) {
				// We handle this in OnAircraftEnteredDropZone
				return GameState->GamePhaseStep;
			}
			else {
				return EAthenaGamePhaseStep::BusLocked;
			}
		}
		else if (GameState->GamePhase == EAthenaGamePhase::SafeZones) {
			if (!GameState->SafeZoneIndicator) {
				return EAthenaGamePhaseStep::StormForming;
			}
			else if (GameState->SafeZoneIndicator->bPaused) {
				return EAthenaGamePhaseStep::StormHolding;
			}
			else {
				return EAthenaGamePhaseStep::StormShrinking;
			}
		}
		else if (GameState->GamePhase == EAthenaGamePhase::EndGame) {
			return EAthenaGamePhaseStep::EndGame;
		}
		else if (GameState->GamePhase == EAthenaGamePhase::Count) {
			return EAthenaGamePhaseStep::Count;
		}
		else {
			return EAthenaGamePhaseStep::EAthenaGamePhaseStep_MAX;
		}
	}

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
			if (GameMode && GameState && UKismetMathLibrary::RandomBool()) {
				EAthenaGamePhaseStep CurrentGamePhaseStep = GetCurrentGamePhaseStep(GameMode, GameState);
				if (CurrentGamePhaseStep != GameState->GamePhaseStep) {
					GameState->GamePhaseStep = CurrentGamePhaseStep;
					if (Globals::bBotsEnabled && Globals::bBotsShouldUseManualTicking) {
						for (FortAthenaAIBotController::BotSpawnData& SpawnedBot : FortAthenaAIBotController::SpawnedBots) {
							if (!SpawnedBot.Controller || !SpawnedBot.Pawn || !SpawnedBot.PlayerState)
								continue;

							SpawnedBot.Controller->Blackboard->SetValueAsEnum(UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep"), (int)GameState->GamePhaseStep);
						}
					}
				}
			}

			if (GameState->GamePhase == EAthenaGamePhase::Warmup &&
				GameMode->AlivePlayers.Num() > 0
				&& (GameMode->AlivePlayers.Num() + GameMode->AliveBots.Num()) < GameMode->GameSession->MaxPlayers
				&& Globals::bBotsEnabled)
			{
				if (UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.045f))
				{
					BotSpawner::SpawnPlayerBot();
				}
			}

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

			if (Globals::bBotsEnabled && Globals::bBotsShouldUseManualTicking) {
				Npcs::TickBots();
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