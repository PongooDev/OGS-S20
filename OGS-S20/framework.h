#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <intrin.h>
#include <sstream>
#include <array>
#include <tlhelp32.h>
#include <future>
#include <set>
#include <algorithm>
#include "minhook/MinHook.h"
#include "SDK/SDK.hpp"

template <class X, class Y>
using xmap = std::map<X, Y, std::less<X>, std::allocator<std::pair<const X, Y>>>;

#pragma comment(lib, "minhook/minhook.lib")

using namespace SDK;

static auto ImageBase = InSDKUtils::GetImageBase();

static void* (*GetWorldContextFromObject)(UEngine*, UWorld*) = decltype(GetWorldContextFromObject)(ImageBase + 0xF1D080);

static UNetDriver* (*CreateNetDriver)(UEngine*, void*, FName) = decltype(CreateNetDriver)(ImageBase + 0x17B0EA0);
static bool (*InitListen)(void*, void*, FURL&, bool, FString&) = decltype(InitListen)(ImageBase + 0x567d81c);
static void (*SetWorld)(void*, void*) = decltype(SetWorld)(ImageBase + 0x17ada40);
static bool (*InitHost)(UObject* Beacon) = decltype(InitHost)(ImageBase + 0x567d4a4);
static void (*PauseBeaconRequests)(UObject* Beacon, bool bPause) = decltype(PauseBeaconRequests)(ImageBase + 0x6d00a04);

static void(*GiveAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) = decltype(GiveAbility)(ImageBase + 0x5250bd8);
static void (*AbilitySpecConstructor)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*) = decltype(AbilitySpecConstructor)(ImageBase + 0x5247bc8);
static bool (*InternalTryActivateAbility)(UAbilitySystemComponent* AbilitySystemComp, FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) = decltype(InternalTryActivateAbility)(ImageBase + 0x5251efc);
static FGameplayAbilitySpecHandle(*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec, void*) = decltype(GiveAbilityAndActivateOnce)(ImageBase + 0x5250CF8);

static FVector* (*PickSupplyDropLocationOG)(AFortAthenaMapInfo* MapInfo, FVector* outLocation, __int64 Center, float Radius) = decltype(PickSupplyDropLocationOG)(ImageBase + 0x6593754);

inline static ABuildingSMActor* (*ReplaceBuildingActor)(ABuildingSMActor* BuildingSMActor, unsigned int a2, UObject* a3, unsigned int a4, int a5, bool bMirrored, AFortPlayerControllerAthena* PC) = decltype(ReplaceBuildingActor)(ImageBase + 0x684A4EC);
static __int64 (*CantBuild)(UWorld*, UObject*, FVector, FRotator, char, void*, char*) = decltype(CantBuild)(ImageBase + 0x6AC00C4);

static void* (*ApplyCharacterCustomization)(AFortPlayerStateAthena* a1, APawn* a2) = decltype(ApplyCharacterCustomization)(ImageBase + 0x6EEC570);

static void (*UnEquipVehicleWeapon)(UFortVehicleSeatWeaponComponent* SeatWeaponComponent, AFortPlayerPawn* Pawn, FWeaponSeatDefinition* WeaponSeatDefinition, bool bEquipBestWeapon) = decltype(UnEquipVehicleWeapon)(ImageBase + 0x70C5BF8);

static void* (*StaticFindObjectOG)(UClass*, UObject* Package, const wchar_t* OrigInName, bool ExactClass) = decltype(StaticFindObjectOG)(ImageBase + 0xf14d30);
static void* (*StaticLoadObjectOG)(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, UObject* Sandbox, bool bAllowObjectReconciliation, void*) = decltype(StaticLoadObjectOG)(ImageBase + 0x1a34ba8);

static TArray<AActor*> PlayerStarts;

// text manipulation utils
namespace TextManipUtils {
	// Found this from stack overflow :fire:
	std::vector<std::string> SplitWhitespace(std::string const& input) {
		std::istringstream buffer(input);
		std::vector<std::string> ret;

		std::copy(std::istream_iterator<std::string>(buffer),
			std::istream_iterator<std::string>(),
			std::back_inserter(ret));
		return ret;
	}
}

void Log(const std::string& msg)
{
	static bool firstCall = true;

	if (firstCall)
	{
		std::ofstream logFile("OGS_log.txt", std::ios::trunc);
		if (logFile.is_open())
		{
			logFile << "[OGS]: Log file initialized!\n";
			logFile.close();
		}
		firstCall = false;
	}

	std::ofstream logFile("OGS_log.txt", std::ios::app);
	if (logFile.is_open())
	{
		logFile << "[OGS]: " << msg << std::endl;
		logFile.close();
	}

	std::cout << "[OGS]: " << msg << std::endl;
}

void HookVTable(void* Base, int Idx, void* Detour, void** OG)
{
	DWORD oldProtection;

	void** VTable = *(void***)Base;

	if (OG)
	{
		*OG = VTable[Idx];
	}

	VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

	VTable[Idx] = Detour;

	VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
}

inline FQuat RotatorToQuat(FRotator Rotation)
{
	FQuat Quat;
	const float DEG_TO_RAD = 3.14159f / 180.0f;
	const float DIVIDE_BY_2 = DEG_TO_RAD / 2.0f;

	float SP = sin(Rotation.Pitch * DIVIDE_BY_2);
	float CP = cos(Rotation.Pitch * DIVIDE_BY_2);
	float SY = sin(Rotation.Yaw * DIVIDE_BY_2);
	float CY = cos(Rotation.Yaw * DIVIDE_BY_2);
	float SR = sin(Rotation.Roll * DIVIDE_BY_2);
	float CR = cos(Rotation.Roll * DIVIDE_BY_2);

	Quat.X = CR * SP * SY - SR * CP * CY;
	Quat.Y = -CR * SP * CY - SR * CP * SY;
	Quat.Z = CR * CP * SY - SR * SP * CY;
	Quat.W = CR * CP * CY + SR * SP * SY;

	return Quat;
}

template <typename T>
static inline T* StaticFindObject(std::wstring ObjectName)
{
	return (T*)StaticFindObjectOG(T::StaticClass(), nullptr, ObjectName.c_str(), false);
}

template<typename T>
inline T* Cast(UObject* Object)
{
	if (!Object || !Object->IsA(T::StaticClass()))
		return nullptr;
	return (T*)Object;
}

template<typename T = UObject>
static inline T* StaticLoadObject(const std::string& Name)
{
	auto ConvName = std::wstring(Name.begin(), Name.end());

	T* Object = StaticFindObject<T>(ConvName);

	if (!Object)
	{
		Object = (T*)StaticLoadObjectOG(T::StaticClass(), nullptr, ConvName.c_str(), nullptr, 0, nullptr, false, nullptr);
	}

	return Object;
}

template <typename _Ot = void*>
__forceinline static void ExecHook(UFunction* _Fn, void* _Detour, _Ot& _Orig = nullptr)
{
	if (!_Fn)
		return;
	if (!std::is_same_v<_Ot, void*>)
		_Orig = (_Ot)_Fn->ExecFunction;

	_Fn->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(_Detour);
}


template <typename _Ot = void*>
__forceinline static void ExecHook(const char* _Name, void* _Detour, _Ot& _Orig = nullptr)
{
	UFunction* _Fn = UObject::FindObject<UFunction>(_Name);
	ExecHook(_Fn, _Detour, _Orig);
}

class FOutputDevice
{
public:
	bool bSuppressEventTag;
	bool bAutoEmitLineTerminator;
	uint8_t _Padding1[0x6];
};

class FFrame : public FOutputDevice
{
public:
	void** VTable;
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;
	void* MostRecentProperty;
	uint8_t* MostRecentPropertyAddress;
	uint8_t _Padding1[0x40];
	FField* PropertyChainForCompiledIn;

public:
	void StepCompiledIn(void* const Result, bool ForceExplicitProp = false)
	{
		if (Code && !ForceExplicitProp)
		{
			((void (*)(FFrame*, UObject*, void* const))(ImageBase + 0xF6D050))(this, Object, Result);
		}
		else
		{
			FField* _Prop = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = _Prop->Next;
			((void (*)(FFrame*, void* const, FField*))(ImageBase + 0x10A04C0))(this, Result, _Prop);
		}
	}

	template <typename T>
	T& StepCompiledInRef()
	{
		T TempVal{};
		MostRecentPropertyAddress = nullptr;

		if (Code)
		{
			((void (*)(FFrame*, UObject*, void* const))(ImageBase + 0xF6D050))(this, Object, &TempVal);
		}
		else
		{
			FField* _Prop = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = _Prop->Next;
			((void (*)(FFrame*, void* const, FField*))(ImageBase + 0x10A04C0))(this, &TempVal, _Prop);
		}

		return MostRecentPropertyAddress ? *(T*)MostRecentPropertyAddress : TempVal;
	}
};

template <typename _It>
static _It* GetInterface(UObject* Object)
{
	return ((_It * (*)(UObject*, UClass*)) (ImageBase + 0xD80568))(Object, _It::StaticClass());
}

template<typename T>
T* GetDefaultObject()
{
	return (T*)T::StaticClass()->DefaultObject;
}

static inline FQuat FRotToQuat(FRotator Rotation) {
	FQuat Quat;
	const float DEG_TO_RAD = 3.14159f / 180.0f;
	const float DIVIDE_BY_2 = DEG_TO_RAD / 2.0f;

	float SP = sin(Rotation.Pitch * DIVIDE_BY_2);
	float CP = cos(Rotation.Pitch * DIVIDE_BY_2);
	float SY = sin(Rotation.Yaw * DIVIDE_BY_2);
	float CY = cos(Rotation.Yaw * DIVIDE_BY_2);
	float SR = sin(Rotation.Roll * DIVIDE_BY_2);
	float CR = cos(Rotation.Roll * DIVIDE_BY_2);

	Quat.X = CR * SP * SY - SR * CP * CY;
	Quat.Y = -CR * SP * CY - SR * CP * SY;
	Quat.Z = CR * CP * SY - SR * SP * CY;
	Quat.W = CR * CP * CY + SR * SP * SY;

	return Quat;
}

template<typename T>
inline T* SpawnActor(FVector Loc, FRotator Rot = FRotator(), AActor* Owner = nullptr, SDK::UClass* Class = T::StaticClass(), ESpawnActorCollisionHandlingMethod Handle = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
	FTransform Transform{};
	Transform.Scale3D = FVector{ 1,1,1 };
	Transform.Translation = Loc;
	Transform.Rotation = FRotToQuat(Rot);

	return (T*)UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, Handle, Owner), Transform);
}

template<typename T>
static inline T* SpawnAActor(FVector Loc = { 0,0,0 }, FRotator Rot = { 0,0,0 }, AActor* Owner = nullptr)
{
	FTransform Transform{};
	Transform.Scale3D = { 1,1,1 };
	Transform.Translation = Loc;
	Transform.Rotation = FRotToQuat(Rot);

	AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), T::StaticClass(), Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
	return (T*)UGameplayStatics::FinishSpawningActor(NewActor, Transform);
}

template<typename T>
static inline T* SpawnActorClass(FVector Loc = { 0,0,0 }, FRotator Rot = { 0,0,0 }, UClass* Class = nullptr, AActor* Owner = nullptr)
{
	FTransform Transform{};
	Transform.Scale3D = { 1,1,1 };
	Transform.Translation = Loc;
	Transform.Rotation = RotatorToQuat(Rot);

	AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
	return (T*)UGameplayStatics::FinishSpawningActor(NewActor, Transform);
}

template<typename T>
T* Actors(UClass* Class = T::StaticClass(), FVector Loc = {}, FRotator Rot = {}, AActor* Owner = nullptr)
{
	return SpawnActor<T>(Loc, Rot, Owner, Class);
}

AFortPickup* SpawnPickup(UFortItemDefinition* ItemDef, int OverrideCount, int LoadedAmmo, FVector Loc, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, bool bShouldCombine = false, AFortPawn* Pawn = nullptr)
{
	auto SpawnedPickup = Actors<AFortPickup>(AFortPickup::StaticClass(), Loc);
	SpawnedPickup->bRandomRotation = true;

	auto& PickupEntry = SpawnedPickup->PrimaryPickupItemEntry;
	PickupEntry.ItemDefinition = ItemDef;
	PickupEntry.Count = OverrideCount;
	PickupEntry.LoadedAmmo = LoadedAmmo;
	PickupEntry.ReplicationKey++;
	SpawnedPickup->OnRep_PrimaryPickupItemEntry();
	SpawnedPickup->PawnWhoDroppedPickup = Pawn;

	SpawnedPickup->TossPickup(Loc, Pawn, -1, true, bShouldCombine, SourceType, Source);

	SpawnedPickup->SetReplicateMovement(true);
	SpawnedPickup->MovementComponent = (UProjectileMovementComponent*)GetDefaultObject<UGameplayStatics>()->SpawnObject(UProjectileMovementComponent::StaticClass(), SpawnedPickup);

	if (SourceType == EFortPickupSourceTypeFlag::Container)
	{
		SpawnedPickup->bTossedFromContainer = true;
		SpawnedPickup->OnRep_TossedFromContainer();
	}

	return SpawnedPickup;
}

static AFortPickup* SpawnPickup(FFortItemEntry ItemEntry, FVector Location, EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset)
{
	auto Pickup = SpawnPickup(ItemEntry.ItemDefinition, ItemEntry.Count, ItemEntry.LoadedAmmo, Location, PickupSource, SpawnSource);
	return Pickup;
}

inline void ShowFoundation(ABuildingFoundation* BuildingFoundation) {
	if (!BuildingFoundation)
		return;

	BuildingFoundation->bServerStreamedInLevel = true;
	BuildingFoundation->DynamicFoundationType = EDynamicFoundationType::Static;
	BuildingFoundation->OnRep_ServerStreamedInLevel();

	BuildingFoundation->FoundationEnabledState = EDynamicFoundationEnabledState::Enabled;
	BuildingFoundation->DynamicFoundationRepData.EnabledState = EDynamicFoundationEnabledState::Enabled;
	BuildingFoundation->DynamicFoundationTransform = BuildingFoundation->GetTransform();
	BuildingFoundation->OnRep_DynamicFoundationRepData();
}

FVector PickSupplyDropLocation(SDK::AFortAthenaMapInfo* MapInfo, SDK::FVector Center, float Radius)
{
	if (!PickSupplyDropLocationOG)
		return SDK::FVector(0, 0, 0);

	TArray<FVector> PickedSupplyDropLocations;

	const float MinDistance = 10000.0f;

	for (int i = 0; i < 20; i++)
	{
		SDK::FVector loc = FVector(0, 0, 0);
		PickSupplyDropLocationOG(MapInfo, &loc, (__int64)&Center, Radius);

		bool bTooClose = false;
		for (const auto& other : PickedSupplyDropLocations)
		{
			float dx = loc.X - other.X;
			float dy = loc.Y - other.Y;
			float dz = loc.Z - other.Z;

			float distSquared = dx * dx + dy * dy + dz * dz;

			if (distSquared < MinDistance * MinDistance)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose)
		{
			PickedSupplyDropLocations.Add(loc);
			return loc;
		}
	}

	return SDK::FVector(0, 0, 0);
}

template <typename T = AActor>
static TArray<T*> GetAll(UClass* Class)
{
	TArray<AActor*> ret;
	UGameplayStatics::GetAllActorsOfClass(UWorld::Get(), Class, &ret);
	return ret;
}

template <typename T = AActor>
static TArray<T*> GetAll()
{
	return GetAll<T>(T::StaticClass());
}

template<typename T>
inline std::vector<T*> GetAllObjectsOfClass(UClass* Class = T::StaticClass())
{
	std::vector<T*> Objects{};

	for (int i = 0; i < UObject::GObjects->Num(); ++i)
	{
		UObject* Object = UObject::GObjects->GetByIndex(i);

		if (!Object)
			continue;

		if (Object->GetFullName().contains("Default"))
			continue;

		if (Object->GetFullName().contains("Test"))
			continue;

		if (Object->IsA(Class) && !Object->IsDefaultObject())
		{
			Objects.push_back((T*)Object);
		}
	}

	return Objects;
}

std::map<AFortPickup*, float> PickupLifetimes;
AFortPickupAthena* SpawnStack(APlayerPawn_Athena_C* Pawn, UFortItemDefinition* Def, int Count, bool giveammo = false, int ammo = 0)
{
	auto Statics = (UGameplayStatics*)UGameplayStatics::StaticClass()->DefaultObject;

	FVector Loc = Pawn->K2_GetActorLocation();
	AFortPickupAthena* Pickup = Actors<AFortPickupAthena>(AFortPickupAthena::StaticClass(), Loc);
	Pickup->bReplicates = true;
	PickupLifetimes[Pickup] = Statics->GetTimeSeconds(UWorld::GetWorld());
	Pickup->PawnWhoDroppedPickup = Pawn;
	Pickup->PrimaryPickupItemEntry.Count = Count;
	Pickup->PrimaryPickupItemEntry.ItemDefinition = Def;
	if (giveammo)
	{
		Pickup->PrimaryPickupItemEntry.LoadedAmmo = ammo;
	}
	Pickup->PrimaryPickupItemEntry.ReplicationKey++;

	Pickup->OnRep_PrimaryPickupItemEntry();
	Pickup->TossPickup(Loc, Pawn, 6, true, true, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);

	Pickup->MovementComponent = (UProjectileMovementComponent*)Statics->SpawnObject(UProjectileMovementComponent::StaticClass(), Pickup);
	Pickup->MovementComponent->bReplicates = true;
	((UProjectileMovementComponent*)Pickup->MovementComponent)->SetComponentTickEnabled(true);

	return Pickup;
}

static int32 EvaluateMinMaxPercent(FScalableFloat Min, FScalableFloat Max, int32 Count)
{
	float AmmoSpawnMin = Min.Value, AmmoSpawnMax = Max.Value;
	auto OutVal = (int)(AmmoSpawnMax - AmmoSpawnMin) == 0 ? 0 : Count * (rand() % (int)(AmmoSpawnMax - AmmoSpawnMin));
	OutVal += Count * (100 - (int)AmmoSpawnMax) / 100;
	return OutVal;
}