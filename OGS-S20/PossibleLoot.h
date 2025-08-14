#pragma once
#include "framework.h"

namespace PossibleLoot {
	struct LootItemInfo {
		UFortItemDefinition* ItemDefinition;
		float Probability;
		int LoadedAmmo;
	};

    struct ResourceInfo {
        UFortItemDefinition* Definition;
        int Quantity;
    };

    inline TArray<LootItemInfo> Weapons() {
        static TArray<LootItemInfo> Weapons;
        if (Weapons.Num() == 0) {
            {
                // Assault Rifles
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/RedDotAR/WID_Assault_RedDotAR_Athena_C.WID_Assault_RedDotAR_Athena_C"), 50, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/RedDotAR/WID_Assault_RedDotAR_Athena_UC.WID_Assault_RedDotAR_Athena_UC"), 30, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/RedDotAR/WID_Assault_RedDotAR_Athena_R.WID_Assault_RedDotAR_Athena_R"), 10, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/RedDotAR/WID_Assault_RedDotAR_Athena_VR.WID_Assault_RedDotAR_Athena_VR"), 5, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/RedDotAR/WID_Assault_RedDotAR_Athena_SR.WID_Assault_RedDotAR_Athena_SR"), 1, 30 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/ar/WID_Assault_Recoil_Athena_C.WID_Assault_Recoil_Athena_C"), 50, 35 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/ar/WID_Assault_Recoil_Athena_UC.WID_Assault_Recoil_Athena_UC"), 30, 35 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/ar/WID_Assault_Recoil_Athena_R.WID_Assault_Recoil_Athena_R"), 10, 35 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/ar/WID_Assault_Recoil_Athena_VR.WID_Assault_Recoil_Athena_VR"), 5, 35 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/ar/WID_Assault_Recoil_Athena_SR.WID_Assault_Recoil_Athena_SR"), 1, 35 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/RedDotBurst/WID_Assault_RedDotBurstAR_Athena_C.WID_Assault_RedDotBurstAR_Athena_C"), 50, 20 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/RedDotBurst/WID_Assault_RedDotBurstAR_Athena_UC.WID_Assault_RedDotBurstAR_Athena_UC"), 30, 20 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/RedDotBurst/WID_Assault_RedDotBurstAR_Athena_R.WID_Assault_RedDotBurstAR_Athena_R"), 10, 20 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/RedDotBurst/WID_Assault_RedDotBurstAR_Athena_VR.WID_Assault_RedDotBurstAR_Athena_VR"), 5, 20 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/RedDotBurst/WID_Assault_RedDotBurstAR_Athena_SR.WID_Assault_RedDotBurstAR_Athena_SR"), 1, 20 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreAR/WID_Assault_CoreAR_Athena_C.WID_Assault_CoreAR_Athena_C"), 50, 25 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreAR/WID_Assault_CoreAR_Athena_UC.WID_Assault_CoreAR_Athena_UC"), 30, 25 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreAR/WID_Assault_CoreAR_Athena_R.WID_Assault_CoreAR_Athena_R"), 10, 25 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreAR/WID_Assault_CoreAR_Athena_VR.WID_Assault_CoreAR_Athena_VR"), 5, 25 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreAR/WID_Assault_CoreAR_Athena_SR.WID_Assault_CoreAR_Athena_SR"), 1, 25 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Surgical_Thermal_Athena_R_Ore_T03.WID_Assault_Surgical_Thermal_Athena_R_Ore_T03"), 10, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Surgical_Thermal_Athena_VR_Ore_T03.WID_Assault_Surgical_Thermal_Athena_VR_Ore_T03"), 5, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Surgical_Thermal_Athena_SR_Ore_T03.WID_Assault_Surgical_Thermal_Athena_SR_Ore_T03"), 1, 15 });
            }
        
            {
                // Shotguns
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/BurstShotgun/WID_Shotgun_CoreBurst_Athena_C.WID_Shotgun_CoreBurst_Athena_C"), 50, 5 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/BurstShotgun/WID_Shotgun_CoreBurst_Athena_UC.WID_Shotgun_CoreBurst_Athena_UC"), 30, 5 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/BurstShotgun/WID_Shotgun_CoreBurst_Athena_R.WID_Shotgun_CoreBurst_Athena_R"), 10, 5 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/BurstShotgun/WID_Shotgun_CoreBurst_Athena_VR.WID_Shotgun_CoreBurst_Athena_VR"), 5, 5 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/BurstShotgun/WID_Shotgun_CoreBurst_Athena_SR.WID_Shotgun_CoreBurst_Athena_SR"), 1, 5 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/BreakActionShotgun/WID_Shotgun_Break_Action_Athena_C_Ore_T03.WID_Shotgun_Break_Action_Athena_C_Ore_T03"), 50, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/BreakActionShotgun/WID_Shotgun_Break_Action_Athena_UC_Ore_T03.WID_Shotgun_Break_Action_Athena_UC_Ore_T03"), 30, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/BreakActionShotgun/WID_Shotgun_Break_Action_Athena_R_Ore_T03.WID_Shotgun_Break_Action_Athena_R_Ore_T03"), 10, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/BreakActionShotgun/WID_Shotgun_Break_Action_Athena_VR_Ore_T03.WID_Shotgun_Break_Action_Athena_VR_Ore_T03"), 5, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/BreakActionShotgun/WID_Shotgun_Break_Action_Athena_SR_Ore_T03.WID_Shotgun_Break_Action_Athena_SR_Ore_T03"), 1, 1 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_AutoDrum_Athena_C_Ore_T03.WID_Shotgun_AutoDrum_Athena_C_Ore_T03"), 50, 12 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_AutoDrum_Athena_UC_Ore_T03.WID_Shotgun_AutoDrum_Athena_UC_Ore_T03"), 30, 12 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_AutoDrum_Athena_R_Ore_T03.WID_Shotgun_AutoDrum_Athena_R_Ore_T03"), 10, 12 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_AutoDrum_Athena_VR_Ore_T03.WID_Shotgun_AutoDrum_Athena_VR_Ore_T03"), 5, 12 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_AutoDrum_Athena_SR_Ore_T03.WID_Shotgun_AutoDrum_Athena_SR_Ore_T03"), 1, 12 });
            }

            {
                // SMGs
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_C.WID_SMG_CoreSMG_Athena_C"), 50, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_UC.WID_SMG_CoreSMG_Athena_UC"), 30, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_R.WID_SMG_CoreSMG_Athena_R"), 10, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_VR.WID_SMG_CoreSMG_Athena_VR"), 5, 30 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_SR.WID_SMG_CoreSMG_Athena_SR"), 1, 30 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/SMG/WID_SMG_Recoil_Athena_C.WID_SMG_Recoil_Athena_C"), 50, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/SMG/WID_SMG_Recoil_Athena_UC.WID_SMG_Recoil_Athena_UC"), 30, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/SMG/WID_SMG_Recoil_Athena_R.WID_SMG_Recoil_Athena_R"), 10, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/SMG/WID_SMG_Recoil_Athena_VR.WID_SMG_Recoil_Athena_VR"), 5, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/SMG/WID_SMG_Recoil_Athena_SR.WID_SMG_Recoil_Athena_SR"), 1, 32 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/LMG/WID_Assault_LMG_Powerup_Athena_C.WID_Assault_LMG_Powerup_Athena_C"), 50, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/LMG/WID_Assault_LMG_Powerup_Athena_UC.WID_Assault_LMG_Powerup_Athena_UC"), 30, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/LMG/WID_Assault_LMG_Powerup_Athena_R.WID_Assault_LMG_Powerup_Athena_R"), 10, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/LMG/WID_Assault_LMG_Powerup_Athena_VR.WID_Assault_LMG_Powerup_Athena_VR"), 5, 32 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/LMG/WID_Assault_LMG_Powerup_Athena_SR.WID_Assault_LMG_Powerup_Athena_SR"), 1, 32 });
            }

            {
                // Pistols
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CorePistol/WID_Pistol_CorePistol_Athena_C.WID_Pistol_CorePistol_Athena_C"), 50, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CorePistol/WID_Pistol_CorePistol_Athena_UC.WID_Pistol_CorePistol_Athena_UC"), 30, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CorePistol/WID_Pistol_CorePistol_Athena_R.WID_Pistol_CorePistol_Athena_R"), 10, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CorePistol/WID_Pistol_CorePistol_Athena_VR.WID_Pistol_CorePistol_Athena_VR"), 5, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CorePistol/WID_Pistol_CorePistol_Athena_SR.WID_Pistol_CorePistol_Athena_SR"), 1, 15 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_C_Ore_T02.WID_Pistol_SixShooter_Athena_C_Ore_T02"), 50, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_UC_Ore_T03.WID_Pistol_SixShooter_Athena_UC_Ore_T03"), 30, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_R_Ore_T03.WID_Pistol_SixShooter_Athena_R_Ore_T03"), 10, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_VR_Ore_T03.WID_Pistol_SixShooter_Athena_VR_Ore_T03"), 5, 15 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SixShooter_Athena_SR_Ore_T03.WID_Pistol_SixShooter_Athena_SR_Ore_T03"), 1, 15 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Flintlock_Athena_C.WID_Pistol_Flintlock_Athena_C"), 50, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Flintlock_Athena_UC.WID_Pistol_Flintlock_Athena_UC"), 30, 1 });
            }

            {
                // Snipers
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_R_Ore_T03.WID_Sniper_Heavy_Athena_R_Ore_T03"), 10, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03"), 5, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03"), 1, 1 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSniper/WID_Sniper_CoreSniper_Athena_UC.WID_Sniper_CoreSniper_Athena_UC"), 30, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSniper/WID_Sniper_CoreSniper_Athena_R.WID_Sniper_CoreSniper_Athena_R"), 10, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSniper/WID_Sniper_CoreSniper_Athena_VR.WID_Sniper_CoreSniper_Athena_VR"), 5, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/Weapons/CoreSniper/WID_Sniper_CoreSniper_Athena_SR.WID_Sniper_CoreSniper_Athena_SR"), 1, 3 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/MotherGameplay/Items/ReactorGrade/WID_Sniper_ReactorGrade_Athena_R_Ore_T03.WID_Sniper_ReactorGrade_Athena_R_Ore_T03"), 10, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/MotherGameplay/Items/ReactorGrade/WID_Sniper_ReactorGrade_Athena_VR_Ore_T03.WID_Sniper_ReactorGrade_Athena_VR_Ore_T03"), 5, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/MotherGameplay/Items/ReactorGrade/WID_Sniper_ReactorGrade_Athena_SR_Ore_T03.WID_Sniper_ReactorGrade_Athena_SR_Ore_T03"), 1, 1 });

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/Rifle/WID_Rifle_Powerup_Athena_C.WID_Rifle_Powerup_Athena_C"), 50, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/Rifle/WID_Rifle_Powerup_Athena_UC.WID_Rifle_Powerup_Athena_UC"), 30, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/Rifle/WID_Rifle_Powerup_Athena_R.WID_Rifle_Powerup_Athena_R"), 10, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/Rifle/WID_Rifle_Powerup_Athena_VR.WID_Rifle_Powerup_Athena_VR"), 5, 3 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/CorruptionGameplay/Gameplay/Items/Weapons/PowerUp/Rifle/WID_Rifle_Powerup_Athena_SR.WID_Rifle_Powerup_Athena_SR"), 1, 3 });
            }

            {
                // Explosives

                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/HomingRocket/WIDs/WID_Launcher_HomingRocket_Athena_R.WID_Launcher_HomingRocket_Athena_R"), 10, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/HomingRocket/WIDs/WID_Launcher_HomingRocket_Athena_VR.WID_Launcher_HomingRocket_Athena_VR"), 5, 1 });
                Weapons.Add({ StaticLoadObject<UFortWeaponRangedItemDefinition>("/ResolveGameplay/Items/Guns/HomingRocket/WIDs/WID_Launcher_HomingRocket_Athena_SR.WID_Launcher_HomingRocket_Athena_SR"), 1, 1 });
            }

            {
                // Melee
            }
        }

        return Weapons;
    }

    inline TArray<ResourceInfo> Utility() {
        static TArray<ResourceInfo> Utility;
        if (Utility.Num() == 0) {
            ResourceInfo Bandage;
            Bandage.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage");
            Bandage.Quantity = 5;
            Utility.Add(Bandage);

            ResourceInfo Medkit;
            Medkit.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Medkit/Athena_Medkit.Athena_Medkit");
            Medkit.Quantity = 1;
            Utility.Add(Medkit);

            ResourceInfo MiniShield;
            MiniShield.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");
            MiniShield.Quantity = 3;
            Utility.Add(MiniShield);

            ResourceInfo BigShield;
            BigShield.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields");
            BigShield.Quantity = 1;
            Utility.Add(BigShield);

            ResourceInfo ChugSplash;
            ChugSplash.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ChillBronco/Athena_ChillBronco.Athena_ChillBronco");
            ChugSplash.Quantity = 2;
            Utility.Add(ChugSplash);

            ResourceInfo ShieldKeg;
            ShieldKeg.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/ShieldGenerator/WID_Athena_ShieldGenerator.WID_Athena_ShieldGenerator");
            ShieldKeg.Quantity = 1;
            Utility.Add(ShieldKeg);

            ResourceInfo DanceGrenade;
            DanceGrenade.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/DanceGrenade/Athena_DanceGrenade.Athena_DanceGrenade");
            DanceGrenade.Quantity = 2;
            Utility.Add(DanceGrenade);

            ResourceInfo ShieldBubble;
            ShieldBubble.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/SilverBlazer/Athena_SilverBlazer_V2.Athena_SilverBlazer_V2");
            ShieldBubble.Quantity = 1;
            Utility.Add(ShieldBubble);

            ResourceInfo AirStrike;
            AirStrike.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Airstrike/Athena_AppleSauce.Athena_AppleSauce");
            AirStrike.Quantity = 1;
            Utility.Add(AirStrike);

            ResourceInfo ShockwaveGrenade;
            ShockwaveGrenade.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade");
            ShockwaveGrenade.Quantity = 2;
            Utility.Add(ShockwaveGrenade);

            ResourceInfo Grenade;
            Grenade.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Grenade/Athena_Grenade.Athena_Grenade");
            Grenade.Quantity = 3;
            Utility.Add(Grenade);

            ResourceInfo C4;
            C4.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/C4/Athena_C4.Athena_C4");
            C4.Quantity = 3;
            Utility.Add(C4);

            ResourceInfo MedMist;
            MedMist.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/FlipperGameplay/Items/HealSpray/WID_Athena_HealSpray.WID_Athena_HealSpray");
            MedMist.Quantity = 100;
            Utility.Add(MedMist);

            ResourceInfo ReconScanner;
            ReconScanner.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/MotherGameplay/Items/Scooter/WID_Athena_Mother_Scooter.WID_Athena_Mother_Scooter");
            ReconScanner.Quantity = 1;
            Utility.Add(ReconScanner);

            ResourceInfo RiftToGo;
            RiftToGo.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/RiftItem/Athena_Rift_Item.Athena_Rift_Item");
            RiftToGo.Quantity = 1;
            Utility.Add(RiftToGo);

            ResourceInfo Balloon;
            Balloon.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Balloons/Athena_Balloons_Consumable.Athena_Balloons_Consumable");
            Balloon.Quantity = 10;
            Utility.Add(Balloon);

            ResourceInfo FireflyJar;
            FireflyJar.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/MolotovCocktail/WID_Athena_Grenade_Molotov.WID_Athena_Grenade_Molotov");
            FireflyJar.Quantity = 2;
            Utility.Add(FireflyJar);
        }

        return Utility;
    }

    inline TArray<ResourceInfo> Environmental() {
        static TArray<ResourceInfo> Environmental;
        if (Environmental.Num() == 0) {
            ResourceInfo SmallFry;
            SmallFry.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Flopper/Small/WID_Athena_FlopperSmall.WID_Athena_FlopperSmall");
            SmallFry.Quantity = 1;
            Environmental.Add(SmallFry);

            ResourceInfo Flopper;
            Flopper.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Flopper/WID_Athena_Flopper.WID_Athena_Flopper");
            Flopper.Quantity = 1;
            Environmental.Add(Flopper);

            ResourceInfo ShieldFish;
            ShieldFish.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Flopper/ShieldFlopper/WID_Athena_Flopper_Shield.WID_Athena_Flopper_Shield");
            ShieldFish.Quantity = 1;
            Environmental.Add(ShieldFish);

            ResourceInfo ThermalFish;
            ThermalFish.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Flopper/Thermal/WID_Athena_Flopper_Thermal.WID_Athena_Flopper_Thermal");
            ThermalFish.Quantity = 1;
            Environmental.Add(ThermalFish);

            ResourceInfo MidasFish;
            MidasFish.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Flopper/MechanicFlopper/WID_Athena_MechanicFlopper.WID_Athena_MechanicFlopper");
            MidasFish.Quantity = 1;
            Environmental.Add(MidasFish);

            ResourceInfo RustyCan;
            RustyCan.Definition = StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Bucket/WID_Athena_Bucket_Old.WID_Athena_Bucket_Old");
            RustyCan.Quantity = 1;
            Environmental.Add(RustyCan);
        }

        return Environmental;
    }

    inline TArray<ResourceInfo> Traps() {
        static TArray<ResourceInfo> Traps;
        if (Traps.Num() == 0) {
            ResourceInfo ArmouredWall;
            ArmouredWall.Definition = StaticLoadObject<UFortContextTrapItemDefinition>("/CorruptionGameplay/Gameplay/Items/Traps/ReinforcedTrap/TID_Context_Reinforced_Athena.TID_Context_Reinforced_Athena");
            ArmouredWall.Quantity = 5;
            Traps.Add(ArmouredWall);

            ResourceInfo LaunchPad;
            LaunchPad.Definition = StaticLoadObject<UFortTrapItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
            LaunchPad.Quantity = 1;
            Traps.Add(LaunchPad);
        }

        return Traps;
    }

    inline TArray<ResourceInfo> Ammo() {
        TArray<ResourceInfo> Ammo;
        if (Ammo.Num() == 0) {
            ResourceInfo SniperAmmo;
            SniperAmmo.Definition = StaticLoadObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
            SniperAmmo.Quantity = 6;
            Ammo.Add(SniperAmmo);

            ResourceInfo SMGAmmo;
            SMGAmmo.Definition = StaticLoadObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
            SMGAmmo.Quantity = 18;
            Ammo.Add(SMGAmmo);

            ResourceInfo AssaultAmmo;
            AssaultAmmo.Definition = StaticLoadObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
            AssaultAmmo.Quantity = 10;
            Ammo.Add(AssaultAmmo);

            ResourceInfo ShotgunInfo;
            ShotgunInfo.Definition = StaticLoadObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
            ShotgunInfo.Quantity = 4;
            Ammo.Add(ShotgunInfo);

            ResourceInfo RocketInfo;
            RocketInfo.Definition = StaticLoadObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
            RocketInfo.Quantity = 2;
            Ammo.Add(RocketInfo);
        }

        return Ammo;
    }

    LootItemInfo GetRandomItemByProbability(const TArray<LootItemInfo>& Items)
    {
        float TotalWeight = 0.0f;
        for (auto& Item : Items)
            TotalWeight += Item.Probability;

        float Rand = RandomFloatRange(0.0f, TotalWeight);
        float Accumulated = 0.0f;

        for (auto& Item : Items)
        {
            Accumulated += Item.Probability;
            if (Rand <= Accumulated)
                return Item;
        }

        return LootItemInfo{};
    }

    FFortItemEntry GetRandomWeapon(EFortRarity MinRarity = EFortRarity::Common, EFortRarity MaxRarity = EFortRarity::Legendary) {
        TArray<LootItemInfo> Items = Weapons();

        TArray<LootItemInfo> FilteredItems;
        for (auto& Item : Items) {
            int Rarity = static_cast<int>(Item.ItemDefinition->Rarity);
            if (Rarity >= static_cast<int>(MinRarity) && Rarity <= static_cast<int>(MaxRarity)) {
                FilteredItems.Add(Item);
            }
        }

        LootItemInfo PickedItem = GetRandomItemByProbability(FilteredItems);

        FFortItemEntry ItemEntry{};
        ItemEntry.ItemDefinition = PickedItem.ItemDefinition;
        ItemEntry.LoadedAmmo = PickedItem.LoadedAmmo;
        ItemEntry.Count = 1;

        return ItemEntry;
    }

    FFortItemEntry GetRandomUtility() {
        TArray<ResourceInfo> Items = Utility();
        
        ResourceInfo PickedItem = Items[rand() % Items.Num()];

        FFortItemEntry ItemEntry{};
        ItemEntry.ItemDefinition = PickedItem.Definition;
        ItemEntry.LoadedAmmo = 1;
        ItemEntry.Count = PickedItem.Quantity;

        return ItemEntry;
    }

    FFortItemEntry GetRandomEnvironmental () {
        TArray<ResourceInfo> Items = Environmental();

        ResourceInfo PickedItem = Items[rand() % Items.Num()];

        FFortItemEntry ItemEntry{};
        ItemEntry.ItemDefinition = PickedItem.Definition;
        ItemEntry.LoadedAmmo = 1;
        ItemEntry.Count = PickedItem.Quantity;

        return ItemEntry;
    }

    FFortItemEntry GetRandomTrap() {
        TArray<ResourceInfo> Items = Traps();

        ResourceInfo PickedItem = Items[rand() % Items.Num()];

        FFortItemEntry ItemEntry{};
        ItemEntry.ItemDefinition = PickedItem.Definition;
        ItemEntry.LoadedAmmo = 1;
        ItemEntry.Count = PickedItem.Quantity;

        return ItemEntry;
    }

    FFortItemEntry GetRandomAmmo() {
        TArray<ResourceInfo> AmmoItems = Ammo();
        ResourceInfo AmmoItem = AmmoItems[rand() % AmmoItems.Num()];

        FFortItemEntry ItemEntry{};
        ItemEntry.ItemDefinition = AmmoItem.Definition;
        ItemEntry.LoadedAmmo = 1;
        ItemEntry.Count = AmmoItem.Quantity;

        return ItemEntry;
    }

    int GetClipSize(UFortItemDefinition* ItemDef) {
        if (auto RangedDef = Cast<UFortWeaponRangedItemDefinition>(ItemDef)) {
            auto DataTable = RangedDef->WeaponStatHandle.DataTable;
            auto RowName = RangedDef->WeaponStatHandle.RowName;

            if (DataTable && RowName.ComparisonIndex) {
                auto& RowMap = *(TMap<FName, FFortRangedWeaponStats*>*)(__int64(DataTable) + 0x30);

                for (auto& Pair : RowMap) {
                    FName CurrentRowName = Pair.Key();
                    FFortRangedWeaponStats* PackageData = Pair.Value();

                    if (CurrentRowName == RowName && PackageData) {
                        return PackageData->ClipSize;
                    }
                }
            }
        }

        return 0;
    }
}