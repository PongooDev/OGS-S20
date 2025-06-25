#pragma once
#include "framework.h"
#include "Inventory.h"
#include "Abilities.h"
#include "Globals.h"

namespace GameMode {
	uint8 NextIdx = 3;
	int CurrentPlayersOnTeam = 0;
	int MaxPlayersOnTeam = 1;

	inline bool (*ReadyToStartMatchOG)(AFortGameModeAthena* GameMode);
	inline bool ReadyToStartMatch(AFortGameModeAthena* GameMode) {
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		static bool SetupPlaylist = false;
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

			auto TS = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			auto DR = 120.f;

			GameState->WarmupCountdownEndTime = TS + DR;
			GameMode->WarmupCountdownDuration = DR;
			GameState->WarmupCountdownStartTime = TS;
			GameMode->WarmupEarlyCountdownDuration = DR;

			GameState->CurrentPlaylistId = Playlist->PlaylistId;
			GameState->OnRep_CurrentPlaylistId();

			GameState->bGameModeWillSkipAircraft = Playlist->bSkipAircraft;
			GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
			GameState->AirCraftBehavior = Playlist->AirCraftBehavior;
			GameState->OnRep_Aircraft();

			GameState->DefaultParachuteDeployTraceForGroundDistance = 10000;

			for (auto& Level : Playlist->AdditionalLevels)
			{
				bool Success = false;
				Log("Level: " + Level.Get()->Name.ToString());
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr);
				FAdditionalLevelStreamed level{};
				level.bIsServerOnly = false;
				level.LevelName = Level.ObjectID.AssetPathName;
				if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
			}
			for (auto& Level : Playlist->AdditionalLevelsServerOnly)
			{
				bool Success = false;
				Log("Server Level: " + Level.Get()->Name.ToString());
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr);
				FAdditionalLevelStreamed level{};
				level.bIsServerOnly = true;
				level.LevelName = Level.ObjectID.AssetPathName;
				if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
			}
			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
			GameState->OnFinishedStreamingAdditionalPlaylistLevel();

			Log("Setup Playlist!");
		}

		if (!GameState->MapInfo) { // cant listen without map being fully loaded
			return false;
		}

		static bool Listening = false;
		if (!Listening) {
			Listening = true;

			auto Beacon = SpawnActor<AFortOnlineBeaconHost>({});
			Beacon->ListenPort = 7777;
			InitHost(Beacon);
			PauseBeaconRequests(Beacon, false);

			UWorld::GetWorld()->NetDriver = Beacon->NetDriver;
			UWorld::GetWorld()->NetDriver->World = UWorld::GetWorld();
			UWorld::GetWorld()->NetDriver->NetDriverName = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"GameNetDriver");

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

			UCurveTable* AthenaGameDataTable = GameState->AthenaGameDataTable;

			if (AthenaGameDataTable)
			{
				static FName DefaultSafeZoneDamageName = FName(UKismetStringLibrary::Conv_StringToName(FString(L"Default.SafeZone.Damage")));

				for (const auto& [RowName, RowPtr] : ((UDataTable*)AthenaGameDataTable)->RowMap)
				{
					if (RowName != DefaultSafeZoneDamageName)
						continue;

					FSimpleCurve* Row = (FSimpleCurve*)RowPtr;

					if (!Row)
						continue;

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

			Log("Listening: 7777");
			SetConsoleTitleA("OGS 20.40 | Listening: 7777");
		}

		if (GameState->PlayersLeft > 0)
		{
			return true;
		}
		else
		{
			auto TS = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			auto DR = 120.f;

			GameState->WarmupCountdownEndTime = TS + DR;
			GameMode->WarmupCountdownDuration = DR;
			GameState->WarmupCountdownStartTime = TS;
			GameMode->WarmupEarlyCountdownDuration = DR;
		}

		return false;
	}

	inline APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerController* Player, AActor* StartingLoc)
	{
		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Player;
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		auto Transform = StartingLoc->GetTransform();
		auto Pawn = GameMode->SpawnDefaultPawnAtTransform(Player, Transform);

		Abilities::InitAbilitiesForPlayer(PC);

		Pawn->NetUpdateFrequency = 100.f;
		Pawn->MinNetUpdateFrequency = 100.f;
		Pawn->bAlwaysRelevant = true;
		Pawn->bReplicateMovement = true;

		auto SprintCompClass = StaticLoadObject<UClass>("/TacticalSprint/Gameplay/TacticalSprintControllerComponent.TacticalSprintControllerComponent_C");
		auto EnergyCompClass = StaticLoadObject<UClass>("/TacticalSprint/Gameplay/TacticalSprintEnergyComponent.TacticalSprintEnergyComponent_C");

		if (SprintCompClass && EnergyCompClass) {
			UFortPlayerControllerComponent_TacticalSprint* SprintComp = (UFortPlayerControllerComponent_TacticalSprint*)PC->AddComponentByClass(SprintCompClass, false, FTransform(), false);
			UFortPlayerControllerComponent_TacticalSprint* SprintComp2 = (UFortPlayerControllerComponent_TacticalSprint*)Pawn->AddComponentByClass(SprintCompClass, false, FTransform(), false);
			UFortComponent_Energy* EnergyComp = (UFortComponent_Energy*)Pawn->AddComponentByClass(EnergyCompClass, false, FTransform(), false);

			SprintComp->CurrentBoundPlayerPawn = (AFortPlayerPawn*)Pawn;
			SprintComp->SetIsSprinting(true);

			SprintComp2->CurrentBoundPlayerPawn = (AFortPlayerPawn*)Pawn;
			SprintComp2->SetIsSprinting(true);
		}
		else {
			Log("SprintCompClass or EnergyCompClass does not exist!");
		}

		PlayerState->SeasonLevelUIDisplay = PC->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();
		PC->XPComponent->bRegisteredWithQuestManager = true;
		PC->XPComponent->OnRep_bRegisteredWithQuestManager();

		PC->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(PC->Pawn);

		UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);
		PlayerState->OnRep_CharacterData();

		PlayerState->SquadId = PlayerState->TeamIndex - 3;
		PlayerState->OnRep_SquadId();

		FGameMemberInfo Member;
		Member.MostRecentArrayReplicationKey = -1;
		Member.ReplicationID = -1;
		Member.ReplicationKey = -1;
		Member.TeamIndex = PlayerState->TeamIndex;
		Member.SquadId = PlayerState->SquadId;
		Member.MemberUniqueId = PlayerState->UniqueId;

		GameState->GameMemberInfoArray.Members.Add(Member);
		GameState->GameMemberInfoArray.MarkItemDirty(Member);

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

		GameState->OnRep_SafeZoneDamage();
		GameState->OnRep_SafeZoneIndicator();
		GameState->OnRep_SafeZonePhase();

		Pawn->OnRep_ReplicateMovement();
		Pawn->OnRep_ReplicatedMovement();

		Pawn->OnRep_PlayerState();
		Pawn->OnRep_Controller();

		Pawn->ForceNetUpdate();
		PC->ForceNetUpdate();
		PlayerState->ForceNetUpdate();

		ApplyCharacterCustomization(PlayerState, Pawn);

		return Pawn;
		//return (AFortPlayerPawnAthena*)GameMode->SpawnDefaultPawnAtTransform(Player, Transform);
	}

	static inline void (*StartNewSafeZonePhaseOG)(AFortGameModeAthena* GameMode, int32 a2);
	static void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32 a2) {
		auto GameState = AFortGameStateAthena::GetDefaultObj();

		FFortSafeZoneDefinition* SafeZoneDefinition = &GameState->MapInfo->SafeZoneDefinition;

		static bool bFirstCall = false;

		auto Duration = 30.f;
		auto HoldDuration = 10.f;
		static auto DPS = 1.f;
		static int ZoneIndex = 0;

		switch (ZoneIndex) {
		case 0:
			Duration = 105.f;
			HoldDuration = 30.f;
			break;
		case 1:
			Duration = 120.f;
			HoldDuration = 110.f;
			DPS = 2.f;
			break;
		case 2:
			Duration = 90.f;
			HoldDuration = 110.f;
			DPS = 3.f;
			break;
		case 3:
			Duration = 95.f;
			HoldDuration = 95.f;
			DPS = 4.f;
			break;
		case 4:
			Duration = 90.f;
			HoldDuration = 90.f;
			DPS = 5.f;
			break;
		case 5:
			Duration = 50.f;
			HoldDuration = 70.f;
			break;
		case 6:
			Duration = 50.f;
			HoldDuration = 70.f;
			DPS = 10.f;
			break;
		case 7:
			Duration = 50.f;
			HoldDuration = 70.f;
			break;
		case 8:
			Duration = 35.f;
			HoldDuration = 60.f;
			DPS = 10.f;
			break;
		case 9:
			Duration = 20.f;
			HoldDuration = 60.f;
			break;
		case 10:
			Duration = 55.f;
			HoldDuration = 60.f;
			break;
		case 11:
			Duration = 50.f;
			HoldDuration = 60.f;
			break;
		case 12:
			Duration = 80.f;
			HoldDuration = 60.f;
			break;
		default:
			Duration = 15.f;
			HoldDuration = 45.f;
			break;
		}

		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + HoldDuration;
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + Duration;
		GameState->SafeZoneDamage = DPS;
		ZoneIndex++;

		GameState->OnRep_SafeZoneDamage();
		GameState->OnRep_SafeZoneIndicator();
		GameState->OnRep_SafeZonePhase();

		StartNewSafeZonePhaseOG(GameMode, a2);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x660b124), ReadyToStartMatch, (LPVOID*)&ReadyToStartMatchOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x6610804), SpawnDefaultPawnFor, nullptr);

		MH_CreateHook((LPVOID)(ImageBase + 0x66151a8), StartNewSafeZonePhase, (LPVOID*)&StartNewSafeZonePhaseOG);

		Log("Gamemode Hooked!");
	}
}