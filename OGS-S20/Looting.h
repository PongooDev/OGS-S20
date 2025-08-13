#pragma once
#include "framework.h"
#include "PossibleLoot.h"

namespace Looting {
    void SpawnLlamas()
    {
        int LlamasSpawned = 0;
        auto LlamasToSpawn = (rand() % 3) + 3;
        Log(std::string("Spawned ") + std::to_string(LlamasToSpawn) + " Llamas");

        auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

        auto MapInfo = GameState->MapInfo;

        for (int i = 0; i < LlamasToSpawn; i++)
        {
            int Radius = 100000;
            FVector Location = PickSupplyDropLocation(MapInfo, FVector(1, 1, 10000), (float)Radius);

            FRotator RandomYawRotator{};
            RandomYawRotator.Yaw = (float)rand() * 0.010986663f;

            FTransform InitialSpawnTransform{};
            InitialSpawnTransform.Translation = Location;
            InitialSpawnTransform.Rotation = FRotToQuat(RandomYawRotator);
            InitialSpawnTransform.Scale3D = FVector(1, 1, 1);

            auto LlamaStart = SpawnActor<AFortAthenaSupplyDrop>(Location, RandomYawRotator, nullptr, MapInfo->LlamaClass.Get());

            if (!LlamaStart)
                continue;

            auto GroundLocation = LlamaStart->FindGroundLocationAt(InitialSpawnTransform.Translation);

            LlamaStart->K2_DestroyActor();

            auto Llama = SpawnActor<AFortAthenaSupplyDrop>(GroundLocation, RandomYawRotator, nullptr, MapInfo->LlamaClass.Get());

            Llama->bCanBeDamaged = false;

            if (!Llama)
                continue;
            LlamasSpawned++;
        }
    }

    std::vector<FFortItemEntry> PickLootDrops(FName TierGroupName, int recursive = 0)
    {
        static auto Bars = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");
        static auto Wood = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
        static auto Metal = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
        static auto Stone = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");

        static UFortWeaponRangedItemDefinition* FishingRod = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/FloppingRabbit/WID_Athena_FloppingRabbit.WID_Athena_FloppingRabbit");
        static UFortWeaponRangedItemDefinition* ProFishingRod = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/FloppingRabbit/WID_Athena_FloppingRabbit_HighTier.WID_Athena_FloppingRabbit_HighTier");

        static UFortWeaponRangedItemDefinition* OffRoadTires = StaticLoadObject<UFortWeaponRangedItemDefinition>("/ValetMods/Mods/TiresOffRoad/Thrown/ID_ValetMod_Tires_OffRoad_Thrown.ID_ValetMod_Tires_OffRoad_Thrown");
        static UFortWeaponRangedItemDefinition* CowCatcher = StaticLoadObject<UFortWeaponRangedItemDefinition>("/CowCatcherMod/Mods/CowCatcher/ID_ValetMod_CowCatcher.ID_ValetMod_CowCatcher");

        std::vector<FFortItemEntry> LootDrops;
        if (TierGroupName.ToString().contains("Ammo")) {
            for (int i = 0; i < 2; i++) {
                LootDrops.push_back(PossibleLoot::GetRandomAmmo());
            }
            if (TierGroupName.ToString().contains("Large")) {
                LootDrops.push_back(PossibleLoot::GetRandomAmmo());
                LootDrops.push_back(PossibleLoot::GetRandomUtility());
                if (UKismetMathLibrary::RandomBool()) {
                    LootDrops.push_back(PossibleLoot::GetRandomTrap());
                }
            }
        }
        else if (TierGroupName.ToString().contains("AthenaTreasure") || TierGroupName.ToString().contains("FactionChest")) {
            int TimesToLoop = 1;
            if (TierGroupName.ToString().contains("IO") || TierGroupName.ToString().contains("FactionChest")) {
                TimesToLoop = 2;
            }

            for (int i = 0; i < TimesToLoop; i++) {
                {
                    FFortItemEntry Weapon;

                    if (TierGroupName.ToString().contains("IO")) {
                        Weapon = PossibleLoot::GetRandomWeapon(EFortRarity::Rare);
                    }
                    else {
                        Weapon = PossibleLoot::GetRandomWeapon(EFortRarity::Uncommon);
                    }

                    if (Weapon.ItemDefinition) {
                        LootDrops.push_back(Weapon);

                        UFortAmmoItemDefinition* Ammo = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Weapon.ItemDefinition)->GetAmmoWorldItemDefinition_BP();
                        if (Ammo) {
                            FFortItemEntry ItemEntry{};
                            ItemEntry.ItemDefinition = Ammo;
                            ItemEntry.LoadedAmmo = PossibleLoot::GetClipSize(ItemEntry.ItemDefinition);
                            ItemEntry.Count = Ammo->DropCount;

                            LootDrops.push_back(ItemEntry);
                        }
                    }
                }

                if (UKismetMathLibrary::RandomBool()) {
                    if (UKismetMathLibrary::RandomBool()) {
                        LootDrops.push_back(PossibleLoot::GetRandomUtility());
                    }
                    else {
                        LootDrops.push_back(PossibleLoot::GetRandomTrap());
                    }
                }

                UFortItemDefinition* Mats = (rand() % 40 > 20) ? ((rand() % 20 > 10) ? Wood : Stone) : Metal;

                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = Mats;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = 30;

                LootDrops.push_back(ItemEntry);

                if (Bars)
                {
                    FFortItemEntry ItemEntry{};
                    ItemEntry.ItemDefinition = Bars;
                    ItemEntry.LoadedAmmo = 0;
                    ItemEntry.Count = UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(1, 30);

                    LootDrops.push_back(ItemEntry);
                }

            }
        }
        else if (TierGroupName.ToString().contains("RodBox")) {
            if (UKismetMathLibrary::RandomBool()) {
                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = FishingRod;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = 1;

                LootDrops.push_back(ItemEntry);
            }
            else {
                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = ProFishingRod;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = 1;

                LootDrops.push_back(ItemEntry);
            }
        }
        else if (TierGroupName.ToString().contains("CoolerBox")) {
            LootDrops.push_back(PossibleLoot::GetRandomEnvironmental());

            if (UKismetMathLibrary::RandomBool()) {
                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = CowCatcher;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = 1;

                LootDrops.push_back(ItemEntry);
            }
            else {
                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = OffRoadTires;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = 1;

                LootDrops.push_back(ItemEntry);
            }
        }
        else if (TierGroupName.ToString().contains("FoodBox_Produce")) {
            LootDrops.push_back(PossibleLoot::GetRandomEnvironmental());
        }
        else if (TierGroupName.ToString().contains("AthenaIceBox")) {
            LootDrops.push_back(PossibleLoot::GetRandomEnvironmental());
            LootDrops.push_back(PossibleLoot::GetRandomEnvironmental());
        }
        else if (TierGroupName.ToString().contains("AthenaWadStash")) {
            FFortItemEntry ItemEntry{};
            ItemEntry.ItemDefinition = Bars;
            ItemEntry.LoadedAmmo = 0;
            ItemEntry.Count = UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(1, 60);

            LootDrops.push_back(ItemEntry);
        }
        else if (TierGroupName.ToString().contains("FloorLoot") && !TierGroupName.ToString().contains("FloorLoot_Warmup")) {
            float Chance = UKismetMathLibrary::RandomFloat();

            if (Chance <= 0.7f) 
            { 
                FFortItemEntry Weapon = PossibleLoot::GetRandomWeapon(EFortRarity::Common, EFortRarity::Rare);
                LootDrops.push_back(Weapon);

                UFortAmmoItemDefinition* Ammo = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Weapon.ItemDefinition)->GetAmmoWorldItemDefinition_BP();
                if (Ammo) {
                    FFortItemEntry ItemEntry{};
                    ItemEntry.ItemDefinition = Ammo;
                    ItemEntry.LoadedAmmo = PossibleLoot::GetClipSize(ItemEntry.ItemDefinition);
                    ItemEntry.Count = Ammo->DropCount;

                    LootDrops.push_back(ItemEntry);
                }
            }
            else 
            { 
                float SubChance = UKismetMathLibrary::RandomFloat(); 
                if (SubChance <= 0.33f) {
                    LootDrops.push_back(PossibleLoot::GetRandomAmmo());
                }
                else if (SubChance <= 0.66f) {
                    LootDrops.push_back(PossibleLoot::GetRandomUtility());
                }
                else {
                    LootDrops.push_back(PossibleLoot::GetRandomTrap());
                }
            }
        }
        else if (TierGroupName.ToString().contains("FloorLoot_Warmup")) 
        {
            float Chance = UKismetMathLibrary::RandomFloat();

            if (Chance <= 0.7f)
            {
                FFortItemEntry Weapon = PossibleLoot::GetRandomWeapon(EFortRarity::Common, EFortRarity::Rare);
                LootDrops.push_back(Weapon);

                UFortAmmoItemDefinition* Ammo = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Weapon.ItemDefinition)->GetAmmoWorldItemDefinition_BP();
                if (Ammo) {
                    FFortItemEntry ItemEntry{};
                    ItemEntry.ItemDefinition = Ammo;
                    ItemEntry.LoadedAmmo = PossibleLoot::GetClipSize(ItemEntry.ItemDefinition);
                    ItemEntry.Count = Ammo->DropCount;

                    LootDrops.push_back(ItemEntry);
                }
            }
            else
            {
               LootDrops.push_back(PossibleLoot::GetRandomAmmo());  
            }
            
        }

        else {
            Log("TierGroupName: " + TierGroupName.ToString());

            {
                FFortItemEntry ItemEntry{};
                ItemEntry.ItemDefinition = Bars;
                ItemEntry.LoadedAmmo = 0;
                ItemEntry.Count = UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(1, 30);

                LootDrops.push_back(ItemEntry);
            }
        }

        return LootDrops;
    }


    bool SpawnLoot(ABuildingContainer* BuildingContainer) {
        std::string ClassName = BuildingContainer->Class->GetName();

        BuildingContainer->bAlreadySearched = true;
        BuildingContainer->SearchBounceData.SearchAnimationCount++;
        BuildingContainer->OnRep_bAlreadySearched();

        auto SearchLootTierGroup = BuildingContainer->SearchLootTierGroup;
        EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset;

        EFortPickupSourceTypeFlag PickupSourceTypeFlags = EFortPickupSourceTypeFlag::Container;

        static auto Loot_Treasure = UKismetStringLibrary::Conv_StringToName(L"Loot_Treasure");
        static auto Loot_Ammo = UKismetStringLibrary::Conv_StringToName(L"Loot_Ammo");
        static auto Loot_AthenaFloorLoot = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot");
        static auto Loot_AthenaFloorLoot_Warmup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");

        if (SearchLootTierGroup == Loot_AthenaFloorLoot || SearchLootTierGroup == Loot_AthenaFloorLoot_Warmup)
        {
            PickupSourceTypeFlags = EFortPickupSourceTypeFlag::FloorLoot;
        }

        if (SearchLootTierGroup == Loot_Treasure)
        {
            EFortPickupSourceTypeFlag PickupSourceTypeFlags = EFortPickupSourceTypeFlag::Container;

            SearchLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure");
            SpawnSource = EFortPickupSpawnSource::Chest;
        }

        if (SearchLootTierGroup == Loot_Ammo)
        {
            EFortPickupSourceTypeFlag PickupSourceTypeFlags = EFortPickupSourceTypeFlag::Container;

            SearchLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge");
            SpawnSource = EFortPickupSpawnSource::AmmoBox;
        }

        if (ClassName.contains("Tiered_Chest"))
        {
            auto LootDrops = PickLootDrops(SearchLootTierGroup);

            auto CorrectLocation = BuildingContainer->K2_GetActorLocation() + (BuildingContainer->GetActorForwardVector() * BuildingContainer->LootSpawnLocation_Athena.X) + (BuildingContainer->GetActorRightVector() * BuildingContainer->LootSpawnLocation_Athena.Y) + (BuildingContainer->GetActorUpVector() * BuildingContainer->LootSpawnLocation_Athena.Z);

            for (auto& LootDrop : LootDrops)
            {
                SpawnPickup(LootDrop.ItemDefinition, LootDrop.Count, LootDrop.LoadedAmmo, CorrectLocation, PickupSourceTypeFlags, SpawnSource);
            }

            return true;
        }
        else
        {
            auto LootDrops = PickLootDrops(SearchLootTierGroup);

            auto CorrectLocation = BuildingContainer->K2_GetActorLocation() + (BuildingContainer->GetActorForwardVector() * BuildingContainer->LootSpawnLocation_Athena.X) + (BuildingContainer->GetActorRightVector() * BuildingContainer->LootSpawnLocation_Athena.Y) + (BuildingContainer->GetActorUpVector() * BuildingContainer->LootSpawnLocation_Athena.Z);

            for (auto& LootDrop : LootDrops)
            {
                if (LootDrop.Count > 0)
                {
                    SpawnPickup(LootDrop.ItemDefinition, LootDrop.Count, LootDrop.LoadedAmmo, CorrectLocation, PickupSourceTypeFlags, SpawnSource);
                }
            }
        }

        return true;
    }

    void DestroyFloorLootSpawners()
    {
        auto Statics = (UGameplayStatics*)UGameplayStatics::StaticClass()->DefaultObject;

        TArray<AActor*> FloorLootSpawners;
        UClass* SpawnerClass = StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
        Statics->GetAllActorsOfClass(UWorld::GetWorld(), SpawnerClass, &FloorLootSpawners);

        for (size_t i = 0; i < FloorLootSpawners.Num(); i++)
        {
            FloorLootSpawners[i]->K2_DestroyActor();
        }

        FloorLootSpawners.Free();

        SpawnerClass = StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
        Statics->GetAllActorsOfClass(UWorld::GetWorld(), SpawnerClass, &FloorLootSpawners);

        for (size_t i = 0; i < FloorLootSpawners.Num(); i++)
        {
            FloorLootSpawners[i]->K2_DestroyActor();
        }

        FloorLootSpawners.Free();
    }

    void Hook() {
        MH_CreateHook((LPVOID)(ImageBase + 0x6820DC4), SpawnLoot, nullptr);

        Log("Looting Hooked!");
    }
}