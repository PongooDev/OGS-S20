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

			for (int i = 0; i < GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevels.Num(); i++)
			{
				FVector Loc{};
				FRotator Rot{};
				bool bSuccess = false;
				((ULevelStreamingDynamic*)ULevelStreamingDynamic::StaticClass()->DefaultObject)->LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevels[i], Loc, Rot, &bSuccess, FString(), nullptr);
				FAdditionalLevelStreamed NewLevel{};
				NewLevel.LevelName = GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevels[i].ObjectID.AssetPathName;
				NewLevel.bIsServerOnly = false;
				GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			}

			for (int i = 0; i < GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevelsServerOnly.Num(); i++)
			{
				FVector Loc{};
				FRotator Rot{};
				bool bSuccess = false;
				((ULevelStreamingDynamic*)ULevelStreamingDynamic::StaticClass()->DefaultObject)->LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevelsServerOnly[i], Loc, Rot, &bSuccess, FString(), nullptr);
				FAdditionalLevelStreamed NewLevel{};
				NewLevel.LevelName = GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevelsServerOnly[i].ObjectID.AssetPathName;
				NewLevel.bIsServerOnly = true;
				GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			}
			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
			GameState->OnFinishedStreamingAdditionalPlaylistLevel();

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

		return Pawn;
		//return (AFortPlayerPawnAthena*)GameMode->SpawnDefaultPawnAtTransform(Player, Transform);
	}

	void Hook() {
		MH_CreateHook((LPVOID)(ImageBase + 0x660b124), ReadyToStartMatch, (LPVOID*)&ReadyToStartMatchOG);

		MH_CreateHook((LPVOID)(ImageBase + 0x6610804), SpawnDefaultPawnFor, nullptr);

		Log("Gamemode Hooked!");
	}
}