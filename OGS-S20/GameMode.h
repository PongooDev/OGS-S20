#pragma once
#include "framework.h"
#include "Inventory.h"
#include "AbilitySystemComponent.h"
#include "Looting.h"
#include "FortAthenaAIBotController.h"
#include "BotSpawner.h"
#include "Globals.h"

namespace GameMode {
	uint8 NextIdx = 3;
	int CurrentPlayersOnTeam = 0;
	int MaxPlayersOnTeam = 1;

	inline bool (*ReadyToStartMatchOG)(AFortGameModeAthena* GameMode);
	inline bool ReadyToStartMatch(AFortGameModeAthena* GameMode) {
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		float CurrentTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
		float WarmupTime = 60.f;

		static bool SetupPlaylist = false;
		static bool bInitialized = false;
		static bool Listening = false;

		if (!SetupPlaylist) {
			SetupPlaylist = true;
			UFortPlaylistAthena* Playlist;
			if (Globals::bCreativeEnabled) {
				Playlist = StaticLoadObject<UFortPlaylistAthena>("/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2");
			}
			else if (Globals::bEventEnabled) {
				Playlist = StaticLoadObject<UFortPlaylistAthena>("/ArmadilloPlaylist/Playlist/Playlist_Armadillo.Playlist_Armadillo");
			}
			else if (Globals::bZeroBuild) {
				Playlist = StaticLoadObject<UFortPlaylistAthena>("/Game/Athena/Playlists/NoBuildBR/Playlist_NoBuildBR_Solo.Playlist_NoBuildBR_Solo");
			}
			else {
				Playlist = StaticLoadObject<UFortPlaylistAthena>("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
			}
			if (!Playlist) {
				Log("Could not find playlist!");
				return false;
			}
			else {
				Log("Found Playlist!");
			}

			GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
			GameState->CurrentPlaylistInfo.MarkArrayDirty();
			GameState->OnRep_CurrentPlaylistInfo();

			GameMode->CurrentPlaylistName = Playlist->PlaylistName;
			GameMode->WarmupRequiredPlayerCount = 1;

			GameMode->bDBNOEnabled = Playlist->MaxTeamSize > 1;
			GameMode->bAlwaysDBNO = Playlist->MaxTeamSize > 1;
			GameState->bDBNODeathEnabled = Playlist->MaxTeamSize > 1;
			GameState->SetIsDBNODeathEnabled(Playlist->MaxTeamSize > 1);

			NextIdx = Playlist->DefaultFirstTeam;
			MaxPlayersOnTeam = Playlist->MaxSquadSize;

			GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;
			GameMode->GameSession->MaxSpectators = 0;
			GameMode->GameSession->MaxPartySize = Playlist->MaxSquadSize;
			GameMode->GameSession->MaxSplitscreensPerConnection = 2;
			GameMode->GameSession->bRequiresPushToTalk = false;
			GameMode->GameSession->SessionName = UKismetStringLibrary::Conv_StringToName(FString(L"GameSession"));

			GameState->WarmupCountdownEndTime = CurrentTime + WarmupTime;
			GameMode->WarmupCountdownDuration = WarmupTime;
			GameState->WarmupCountdownStartTime = CurrentTime;
			GameMode->WarmupEarlyCountdownDuration = WarmupTime;

			GameState->CurrentPlaylistId = Playlist->PlaylistId;
			GameState->OnRep_CurrentPlaylistId();

			GameState->bGameModeWillSkipAircraft = Playlist->bSkipAircraft;
			GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
			GameState->AirCraftBehavior = Playlist->AirCraftBehavior;
			GameState->OnRep_Aircraft();

			Log("Setup Playlist!");
		}

		if (!GameState->MapInfo) { // cant listen without map being fully loaded
			return false;
		}

		if (!bInitialized) {
			bInitialized = true;

			GameState->DefaultParachuteDeployTraceForGroundDistance = 10000;

			GameMode->AISettings = GameState->CurrentPlaylistInfo.BasePlaylist->AISettings.Get();
			GameMode->AISettings->AIServices[1] = UAthenaAIServicePlayerBots::StaticClass();

			if (!GameMode->SpawningPolicyManager)
			{
				GameMode->SpawningPolicyManager = SpawnActor<AFortAthenaSpawningPolicyManager>({}, {});
			}
			GameMode->SpawningPolicyManager->GameModeAthena = GameMode;
			GameMode->SpawningPolicyManager->GameStateAthena = GameState;

			GameMode->AIDirector = SpawnActor<AAthenaAIDirector>({});
			if (GameMode->AIDirector) {
				GameMode->AIDirector->Activate();
			}
			else {
				Log("No AIDirector!");
			}

			if (!GameMode->AIGoalManager)
			{
				GameMode->AIGoalManager = SpawnActor<AFortAIGoalManager>({});
			}

			UAISystem::GetDefaultObj()->AILoggingVerbose();

			for (size_t i = 0; i < UObject::GObjects->Num(); i++)
			{
				UObject* Obj = UObject::GObjects->GetByIndex(i);
				if (Obj && Obj->IsA(UAthenaCharacterItemDefinition::StaticClass()))
				{
					std::string SkinsData = ((UAthenaCharacterItemDefinition*)Obj)->Name.ToString();

					if (SkinsData.contains("Athena_Commando") || SkinsData.contains("CID_Character") || !SkinsData.contains("CID_NPC") || !SkinsData.contains("CID_VIP") || !SkinsData.contains("CID_TBD"))
					{
						Characters.push_back((UAthenaCharacterItemDefinition*)Obj);
					}
				}
				if (Obj && Obj->IsA(UAthenaBackpackItemDefinition::StaticClass()))
				{
					Backpacks.push_back((UAthenaBackpackItemDefinition*)Obj);
				}
				if (Obj && Obj->IsA(UAthenaPickaxeItemDefinition::StaticClass()))
				{
					Pickaxes.push_back((UAthenaPickaxeItemDefinition*)Obj);
				}
				if (Obj && Obj->IsA(UAthenaDanceItemDefinition::StaticClass()))
				{
					std::string EmoteData = ((UAthenaDanceItemDefinition*)Obj)->Name.ToString();

					if (EmoteData.contains("EID") || !EmoteData.contains("Sync") || !EmoteData.contains("Owned"))
					{
						Dances.push_back((UAthenaDanceItemDefinition*)Obj);
					}

				}
				if (Obj && Obj->IsA(UAthenaGliderItemDefinition::StaticClass()))
				{
					Gliders.push_back((UAthenaGliderItemDefinition*)Obj);
				}
			}

			for (auto& Level : GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevels)
			{
				bool Success = false;
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr);
				FAdditionalLevelStreamed level{};
				level.bIsServerOnly = false;
				level.LevelName = Level.ObjectID.AssetPathName;
				if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
			}
			for (auto& Level : GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevelsServerOnly)
			{
				bool Success = false;
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr);
				FAdditionalLevelStreamed level{};
				level.bIsServerOnly = true;
				level.LevelName = Level.ObjectID.AssetPathName;
				if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
			}
			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
			GameState->OnFinishedStreamingAdditionalPlaylistLevel();

			UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), AFortPlayerStartWarmup::StaticClass(), &PlayerStarts);

			GameState->DefaultBattleBus = StaticLoadObject<UAthenaBattleBusItemDefinition>("/Game/Athena/Items/Cosmetics/BattleBuses/BBID_BirthdayBus4th.BBID_BirthdayBus4th");

			for (size_t i = 0; i < GameMode->BattleBusCosmetics.Num(); i++)
			{
				GameMode->BattleBusCosmetics[i] = GameState->DefaultBattleBus;
			}

			if (Globals::bArenaEnabled)
			{
				GameState->EventTournamentRound = EEventTournamentRound::Arena;
				GameState->OnRep_EventTournamentRound();
			}

			Log("Initialized Game!");
		}

		if (!Listening) {
			Listening = true;

			FName NetDriverName = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"GameNetDriver");
			void* WorldContext = GetWorldContextFromObject(UEngine::GetEngine(), UWorld::GetWorld());
			UNetDriver* NetDriver = CreateNetDriver(UEngine::GetEngine(), WorldContext, NetDriverName);

			NetDriver->NetDriverName = NetDriverName;
			NetDriver->World = UWorld::GetWorld();
			UWorld::GetWorld()->NetDriver = NetDriver;

			FString Error;
			FURL url = FURL();
			url.Port = 7777;

			if (!InitListen(UWorld::GetWorld()->NetDriver, UWorld::GetWorld(), url, true, Error))
			{
				Log("InitListen Failed!");
			}
			else {
				Log("InitListen successful!");
			}

			SetWorld(UWorld::GetWorld()->NetDriver, UWorld::GetWorld());

			for (size_t i = 0; i < UWorld::GetWorld()->LevelCollections.Num(); i++) {
				UWorld::GetWorld()->LevelCollections[i].NetDriver = UWorld::GetWorld()->NetDriver;
			}

			SetWorld(UWorld::GetWorld()->NetDriver, UWorld::GetWorld());

			GameMode->bWorldIsReady = true;

			Log("Listening: 7777");
			SetConsoleTitleA("OGS 20.40 | Listening: 7777");
		}

		if (GameState->PlayersLeft > 0)
		{
			UCurveTable* AthenaGameDataTable = GameState->AthenaGameDataTable;
			if (AthenaGameDataTable)
			{
				static FName DefaultSafeZoneDamageName = FName(UKismetStringLibrary::Conv_StringToName(FString(L"Default.SafeZone.Damage")));

				for (const auto& [RowName, RowPtr] : ((UDataTable*)AthenaGameDataTable)->RowMap)
				{
					if (RowName != DefaultSafeZoneDamageName)
						continue;

					FSimpleCurve* Row = (FSimpleCurve*)RowPtr;
					for (auto& Key : Row->Keys)
					{
						FSimpleCurveKey* KeyPtr = &Key;

						if (KeyPtr->Time == 0.f)
						{
							KeyPtr->Value = 0.f;
						}
					}

					Row->Keys.Add(FSimpleCurveKey(1.f, 0.01f));
				}
			}
			else {
				Log("No AthenaGameDataTable!");
			}

			return true;
		}
		else
		{
			GameState->WarmupCountdownEndTime = CurrentTime + WarmupTime;
			GameMode->WarmupCountdownDuration = WarmupTime;
			GameState->WarmupCountdownStartTime = CurrentTime;
			GameMode->WarmupEarlyCountdownDuration = WarmupTime;
		}

		return false;
	}

	inline APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerController* Player, AActor* StartingLoc)
	{
		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Player;
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		AActor* StartSpot = GameMode->FindPlayerStart(Player, L"");
		auto Transform = StartSpot->GetTransform();
		auto Pawn = GameMode->SpawnDefaultPawnAtTransform(Player, Transform);

		Pawn->NetUpdateFrequency = 100.f;
		Pawn->MinNetUpdateFrequency = 100.f;
		Pawn->bAlwaysRelevant = true;
		Pawn->bReplicateMovement = true;

		PlayerState->SeasonLevelUIDisplay = PC->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();
		PC->XPComponent->bRegisteredWithQuestManager = true;
		PC->XPComponent->OnRep_bRegisteredWithQuestManager();

		PC->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(PC->Pawn);

		UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);
		PlayerState->OnRep_CharacterData();

		UAthenaPickaxeItemDefinition* PickDef;
		FFortAthenaLoadout& CosmecticLoadoutPC = PC->CosmeticLoadoutPC;
		PickDef = CosmecticLoadoutPC.Pickaxe != nullptr ? CosmecticLoadoutPC.Pickaxe : StaticLoadObject<UAthenaPickaxeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		//UFortWeaponMeleeItemDefinition* PickDef = StaticLoadObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		if (PickDef) {
			Log("Pick Does Exist!");
			Inventory::GiveItem(PC, PickDef->WeaponDefinition, 1, 0);
		}
		else {
			Log("Pick Doesent Exist!");
		}

		for (size_t i = 0; i < GameMode->StartingItems.Num(); i++)
		{
			if (GameMode->StartingItems[i].Count > 0)
			{
				Inventory::GiveItem(PC, GameMode->StartingItems[i].Item, GameMode->StartingItems[i].Count, 0);
			}
		}

		Pawn->OnRep_PlayerState();
		Pawn->OnRep_Controller();

		Pawn->ForceNetUpdate();
		PC->ForceNetUpdate();
		PlayerState->ForceNetUpdate();

		ApplyCharacterCustomization(PlayerState, Pawn);

		return Pawn;
		//return (AFortPlayerPawnAthena*)GameMode->SpawnDefaultPawnAtTransform(Player, Transform);
	}

	static inline void (*StartNewSafeZonePhaseOG)(AFortGameModeAthena* GameMode, int32 ZoneIndex);
	static void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32 ZoneIndex) {
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;

		FFortSafeZoneDefinition* SafeZoneDefinition = &GameState->MapInfo->SafeZoneDefinition;
		Log("SafeZonePhase: " + std::to_string(GameMode->SafeZonePhase));

		static UCurveTable* AthenaGameData = StaticLoadObject<UCurveTable>(UKismetStringLibrary::Conv_NameToString(GameState->CurrentPlaylistInfo.BasePlaylist->GameData.ObjectID.AssetPathName).ToString());
		if (!AthenaGameData) {
			AthenaGameData = StaticLoadObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");
		}
		float CurrentWaitTime = 30.f;
		EEvaluateCurveTableResult WaitTimeResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.WaitTime"), (GameMode->SafeZonePhase + 1), &WaitTimeResult, &CurrentWaitTime, FString());
		if (WaitTimeResult == EEvaluateCurveTableResult::RowNotFound) {
			Log("Not Found WaitTime Row!");
		}
		else {
			GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + CurrentWaitTime;
		}

		float CurrentShrinkTime = 30.f;
		EEvaluateCurveTableResult ShrinkTimeResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.ShrinkTime"), (GameMode->SafeZonePhase + 1), &ShrinkTimeResult, &CurrentShrinkTime, FString());
		if (ShrinkTimeResult == EEvaluateCurveTableResult::RowNotFound) {
			Log("Not Found ShrinkTime Row!");
		}
		else {
			GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + CurrentShrinkTime;
		}

		GameState->OnRep_SafeZoneIndicator();
		GameState->OnRep_SafeZonePhase();

		StartNewSafeZonePhaseOG(GameMode, ZoneIndex);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x660b124), ReadyToStartMatch, (LPVOID*)&ReadyToStartMatchOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x6610804), SpawnDefaultPawnFor, nullptr);

		MH_CreateHook((LPVOID)(ImageBase + 0x66151a8), StartNewSafeZonePhase, (LPVOID*)&StartNewSafeZonePhaseOG);

		Log("Gamemode Hooked!");
	}
}