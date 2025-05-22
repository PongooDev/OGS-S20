#pragma once
#include "framework.h"

namespace Replication {
	struct FNetworkObjectInfo
	{
		class SDK::AActor* Actor;

		TWeakObjectPtr<class SDK::AActor> WeakActor;

		double NextUpdateTime;

		double LastNetReplicateTime;

		float OptimalNetUpdateDelta;

		double LastNetUpdateTimestamp;

		TSet<TWeakObjectPtr<class SDK::UNetConnection>> DormantConnections;

		TSet<TWeakObjectPtr<class SDK::UNetConnection>> RecentlyDormantConnections;

		uint8 bPendingNetUpdate : 1;

		uint8 bDirtyForReplay : 1;

		uint8 bSwapRolesOnReplicate : 1;

		uint32 ForceRelevantFrame = 0;
	};


	template< class ObjectType>
	class TSharedPtr
	{
	public:
		ObjectType* Object;

		int32 SharedReferenceCount;
		int32 WeakReferenceCount;

		FORCEINLINE ObjectType* Get()
		{
			return Object;
		}
		FORCEINLINE ObjectType* Get() const
		{
			return Object;
		}
		FORCEINLINE ObjectType& operator*()
		{
			return *Object;
		}
		FORCEINLINE const ObjectType& operator*() const
		{
			return *Object;
		}
		FORCEINLINE ObjectType* operator->()
		{
			return Object;
		}
		FORCEINLINE ObjectType* operator->() const
		{
			return Object;
		}
	};

	class FNetworkObjectList
	{
	public:
		using FNetworkObjectSet = TSet<TSharedPtr<FNetworkObjectInfo>>;

		FNetworkObjectSet AllNetworkObjects;
		FNetworkObjectSet ActiveNetworkObjects;
		FNetworkObjectSet ObjectsDormantOnAllConnections;

		TMap<TWeakObjectPtr<UNetConnection>, int32> NumDormantObjectsPerConnection;

		void Remove(AActor* const Actor);
	};

	struct alignas(0x4) FServerFrameInfo
	{
		int32 LastProcessedInputFrame = -1;
		int32 LastLocalFrame = -1;
		int32 LastSentLocalFrame = -1;

		float TargetTimeDilation = 1.f;
		int8 QuantizedTimeDilation = 1;
		float TargetNumBufferedCmds = 1.f;
		bool bFault = true;
	};

	class FNetworkGUID
	{
	public:

		uint32 Value;

	public:

		FNetworkGUID()
			: Value(0)
		{
		}

		FNetworkGUID(uint32 V)
			: Value(V)
		{
		}

	public:

		friend bool operator==(const FNetworkGUID& X, const FNetworkGUID& Y)
		{
			return (X.Value == Y.Value);
		}

		friend bool operator!=(const FNetworkGUID& X, const FNetworkGUID& Y)
		{
			return (X.Value != Y.Value);
		}
	};


	struct FActorDestructionInfo
	{
	public:
		FActorDestructionInfo()
			: Reason(0)
			, bIgnoreDistanceCulling(false)
		{
		}

		TWeakObjectPtr<class SDK::ULevel> Level;
		TWeakObjectPtr<class SDK::UObject> ObjOuter;
		struct SDK::FVector DestroyedPosition;
		class FNetworkGUID NetGUID;
		class SDK::FString PathName;
		class SDK::FName StreamingLevelName;
		uint8_t Reason;

		bool bIgnoreDistanceCulling;
	};

	template< class ObjectType>
	class TUniquePtr
	{
	public:
		ObjectType* Ptr;

		FORCEINLINE ObjectType* Get()
		{
			return Ptr;
		}
		FORCEINLINE ObjectType* Get() const
		{
			return Ptr;
		}
		FORCEINLINE ObjectType& operator*()
		{
			return *Ptr;
		}
		FORCEINLINE const ObjectType& operator*() const
		{
			return *Ptr;
		}
		FORCEINLINE ObjectType* operator->()
		{
			return Ptr;
		}
		FORCEINLINE ObjectType* operator->() const
		{
			return Ptr;
		}
	};

	#define CLOSEPROXIMITY					500.
	#define NEARSIGHTTHRESHOLD				2000.
	#define MEDSIGHTTHRESHOLD				3162.
	#define FARSIGHTTHRESHOLD				8000.
	#define CLOSEPROXIMITYSQUARED			(CLOSEPROXIMITY*CLOSEPROXIMITY)
	#define NEARSIGHTTHRESHOLDSQUARED		(NEARSIGHTTHRESHOLD*NEARSIGHTTHRESHOLD)
	#define MEDSIGHTTHRESHOLDSQUARED		(MEDSIGHTTHRESHOLD*MEDSIGHTTHRESHOLD)
	#define FARSIGHTTHRESHOLDSQUARED		(FARSIGHTTHRESHOLD*FARSIGHTTHRESHOLD)

	enum class EChannelCreateFlags : uint32_t
	{
		None = (1 << 0),
		OpenedLocally = (1 << 1)
	};

	__forceinline int& GetReplicationFrame(UNetDriver* Driver)
	{
		return *(int*)(__int64(Driver) + 0x3d8);
	}

	__forceinline FNetworkObjectList& GetNetworkObjectList(UNetDriver* Driver)
	{
		return *(*(TSharedPtr<FNetworkObjectList>*)(__int64(Driver) + 0x6b8));
	}

	__forceinline double& GetElapsedTime(UNetDriver* Driver)
	{
		return *(double*)(__int64(Driver) + 0x218);
	}

	__forceinline AActor* GetViewTarget(APlayerController* Controller)
	{
		if (!Controller)
			return nullptr;

		using GetViewTargetFn = AActor * (__fastcall*)(APlayerController*);
		auto VTable = *(uintptr_t**)Controller;
		auto Fn = (GetViewTargetFn)VTable[0xE8];

		return Fn ? Fn(Controller) : nullptr;
	}

	__forceinline double& GetRelevantTime(UActorChannel* Channel)
	{
		return *(double*)(__int64(Channel) + 0x78);
	}

	__forceinline int32& GetNetTag(UNetDriver* Driver)
	{
		return *(int32*)(__int64(Driver) + 0x2e4);
	}

	__forceinline TSet<FNetworkGUID>& GetDestroyedStartupOrDormantActorGUIDs(UNetConnection* Conn)
	{
		return *(TSet<FNetworkGUID>*)(__int64(Conn) + 0x1488);
	}

	__forceinline TMap<FNetworkGUID, TUniquePtr<FActorDestructionInfo>>& GetDestroyedStartupOrDormantActors(UNetDriver* Driver)
	{
		return *(TMap<FNetworkGUID, TUniquePtr<FActorDestructionInfo>>*)(__int64(Driver) + 0x2e8);
	}

	__forceinline uint32& GetLastProcessedFrame(UNetConnection* Conn)
	{
		return *(uint32*)(__int64(Conn) + 0x200);
	}

	__forceinline TSet<FName>& GetClientVisibleLevelNames(UNetConnection* Conn)
	{
		return *(TSet<FName>*)(__int64(Conn) + 0x1578);
	}

	__forceinline FName& GetClientWorldPackageName(UNetConnection* Conn)
	{
		return *(FName*)(__int64(Conn) + 0x16b8);
	}

	__forceinline AActor*& GetRepContextActor(UNetConnection* Conn)
	{
		return *(AActor**)(__int64(Conn) + 0x16c8);
	}

	__forceinline ULevel*& GetRepContextLevel(UNetConnection* Conn)
	{
		return *(ULevel**)(__int64(Conn) + 0x16d0);
	}

	__forceinline TSharedPtr<void*>& GetGuidCache(UNetDriver* Driver)
	{
		return *(TSharedPtr<void*>*)(__int64(Driver) + 0x150);
	}

	__forceinline bool& GetPendingCloseDueToReplicationFailure(UNetConnection* Conn)
	{
		return *(bool*)(__int64(Conn) + 0x1b56);
	}


	int PrepConns(UNetDriver* Driver)
	{
		if (!Driver) return 0;

		for (auto& Conn : Driver->ClientConnections)
		{
			if (!Conn || !Conn->Driver) continue;

			auto Owner = Conn->OwningActor;
			if (Owner && GetElapsedTime(Conn->Driver) - Conn->LastReceiveTime < 1.5)
			{
				auto OutViewTarget = Owner;
				if (auto Controller = Conn->PlayerController)
					if (auto ViewTarget = GetViewTarget(Controller))
						OutViewTarget = ViewTarget;

				Conn->ViewTarget = OutViewTarget;

				for (auto& Child : Conn->Children)
				{
					if (Child && Child->PlayerController)
						Child->ViewTarget = GetViewTarget(Child->PlayerController);
					else if (Child)
						Child->ViewTarget = nullptr;
				}
			}
			else
			{
				if (Conn) Conn->ViewTarget = nullptr;
				for (auto& Child : Conn->Children)
					if (Child)
						Child->ViewTarget = nullptr;
			}
		}

		return Driver->ClientConnections.Num();
	}

	__forceinline float FRand()
	{
		/*random_device rd;
		mt19937 gen(rd());
		uniform_real_distribution<> dis(0, 1);
		float random_number = (float) dis(gen);

		return random_number;*/
		constexpr int32 RandMax = 0x00ffffff < RAND_MAX ? 0x00ffffff : RAND_MAX;
		return (rand() & RandMax) / (float)RandMax;
	}

	float fastLerp(float a, float b, float t)
	{
		//return (a * (1.f - t)) + (b * t);
		return a - (a + b) * t;
	}

	TArray<FNetworkObjectInfo*> ConsiderList;
	__forceinline void BuildConsiderList(UNetDriver* Driver, float DeltaTime)
	{
		auto& ActiveNetworkObjects = GetNetworkObjectList(Driver).ActiveNetworkObjects;
		ConsiderList.Clear();
		ConsiderList.Reserve(ActiveNetworkObjects.Num());
		auto Time = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
		TArray<AActor*> ActorsToRemove;
		for (auto& ActorInfo : ActiveNetworkObjects)
		{
			if (!ActorInfo->bPendingNetUpdate && Time <= ActorInfo->NextUpdateTime)
				continue;
			auto Actor = ActorInfo->Actor;
			auto Outer = Actor->Outer;
			if (Actor->bActorIsBeingDestroyed || Actor->RemoteRole == ENetRole::ROLE_None || Actor->NetDriverName != Driver->NetDriverName || (Actor->bNetStartup && Actor->NetDormancy == ENetDormancy::DORM_Initial))
			{
				ActorsToRemove.Add(Actor);
				continue;
			}

			ULevel* Level = nullptr;
			while (Outer && !Level)
			{
				if (Outer->Class == ULevel::StaticClass())
				{
					Level = (ULevel*)Outer;
					break;
				}
				else
				{
					Outer = Outer->Outer;
				}
			}
			if (Level == Level->OwningWorld->CurrentLevelPendingVisibility || Level == Level->OwningWorld->CurrentLevelPendingInvisibility)
			{
				continue;
			}

			if (ActorInfo->LastNetReplicateTime == 0)
			{
				ActorInfo->LastNetReplicateTime = Time;
				//ActorInfo->OptimalNetUpdateDelta = 1.f / Actor->NetUpdateFrequency;
			}

			/*const float LastReplicateDelta = float(Time - ActorInfo->LastNetReplicateTime);

			if (LastReplicateDelta > 2.f)
			{
				if (Actor->MinNetUpdateFrequency == 0.f)
				{
					Actor->MinNetUpdateFrequency = 2.f;
				}

				const float MinOptimalDelta = 1.f / Actor->NetUpdateFrequency;
				const float MaxOptimalDelta = max(1.f / Actor->MinNetUpdateFrequency, MinOptimalDelta);

				const float Alpha = clamp((LastReplicateDelta - 2.f) / 5.f, 0.f, 1.f);
				ActorInfo->OptimalNetUpdateDelta = fastLerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
			}*/

			if (!ActorInfo->bPendingNetUpdate)
			{
				//constexpr bool bUseAdapativeNetFrequency = false;
				const float NextUpdateDelta = /*bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : */1.f / Actor->NetUpdateFrequency;

				float ServerTickTime = 1.f / std::clamp(1.f / DeltaTime, 1.f, 30.f);
				ActorInfo->NextUpdateTime = Time + FRand() * ServerTickTime + NextUpdateDelta;
				ActorInfo->LastNetUpdateTimestamp = (float)GetElapsedTime(Driver);
			}
			else
			{
				ActorInfo->bPendingNetUpdate = false;
			}

			ConsiderList.Add(ActorInfo.Get());
		}

		ActorsToRemove.Free();
	}

	__forceinline bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
	{
		/*using IsNetRelevantForFn = bool(*)(const AActor*, const AActor*, const AActor*, const FVector&);
		IsNetRelevantForFn IsNetRelevantFor = (IsNetRelevantForFn)(Actor->VTable[0x9a]);

		for (auto& Viewer : ConnectionViewers)
		{
			if (IsNetRelevantFor(Actor, Viewer.InViewer, Viewer.ViewTarget, Viewer.ViewLocation))
			{
				return true;
			}
		}

		return false;*/
		return true;
	}

	__forceinline FNetViewer ConstructNetViewer(UNetConnection* NetConnection)
	{
		FNetViewer newViewer{};
		newViewer.Connection = NetConnection;
		newViewer.InViewer = NetConnection->PlayerController ? NetConnection->PlayerController : NetConnection->OwningActor;
		newViewer.ViewTarget = NetConnection->ViewTarget;

		APlayerController* ViewingController = NetConnection->PlayerController;

		newViewer.ViewLocation = newViewer.ViewTarget->K2_GetActorLocation();

		if (ViewingController)
		{
			FRotator ViewRotation = ViewingController->ControlRotation;
			constexpr auto radian = 0.017453292519943295;
			double cosPitch = cos(ViewRotation.Pitch * radian), sinPitch = sin(ViewRotation.Pitch * radian), cosYaw = cos(ViewRotation.Yaw * radian), sinYaw = sin(ViewRotation.Yaw * radian);
			newViewer.ViewDir = FVector(cosPitch * cosYaw, cosPitch * sinYaw, sinPitch);
		}

		return newViewer;
	}

	__forceinline bool ShouldActorGoDormant(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth)
	{
		if (Actor->NetDormancy <= ENetDormancy::DORM_Awake || !Channel)
		{
			// Either shouldn't go dormant, or is already dormant
			return false;
		}

		if (Actor->NetDormancy == ENetDormancy::DORM_DormantPartial)
		{
			for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++)
			{
				//if (!GetNetDormancy(Actor, ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
				{
					return false;
				}
			}
		}

		return true;
	}

	__forceinline bool IsLevelInitializedForActor(const UNetDriver* NetDriver, const AActor* InActor, UNetConnection* InConnection)
	{
		/*bool (*ClientHasInitializedLevelFor)(const UNetConnection*, const AActor*) = decltype(ClientHasInitializedLevelFor)();

		const bool bCorrectWorld = NetDriver->WorldPackage != nullptr && (GetClientWorldPackageName(InConnection) == NetDriver->WorldPackage->Name) && ClientHasInitializedLevelFor(InConnection, InActor);
		const bool bIsConnectionPC = (InActor == InConnection->PlayerController);
		return bCorrectWorld || bIsConnectionPC;*/
		return true;
	}

	__forceinline void SendClientAdjustment(APlayerController* PlayerController)
	{
		auto& ServerFrameInfo = *(FServerFrameInfo*)(__int64(PlayerController) + 0x7a4);

		if (ServerFrameInfo.LastProcessedInputFrame != -1 && ServerFrameInfo.LastProcessedInputFrame != ServerFrameInfo.LastSentLocalFrame)
		{
			ServerFrameInfo.LastSentLocalFrame = ServerFrameInfo.LastProcessedInputFrame;
			PlayerController->ClientRecvServerAckFrame(ServerFrameInfo.LastProcessedInputFrame, ServerFrameInfo.LastLocalFrame, ServerFrameInfo.QuantizedTimeDilation);
		}

		auto Pawn = (ACharacter*)PlayerController->Pawn;

		if (!Pawn || Pawn->RemoteRole != ENetRole::ROLE_AutonomousProxy)
		{
			return;
		}

		/*auto Interface = Utils::GetInterface<INetworkPredictionInterface>(Pawn->CharacterMovement);

		if (Interface)
		{
			auto SendClientAdjustmemt = (void (*)(INetworkPredictionInterface*)) Sarah::Offsets::SendClientAdjustment;
			SendClientAdjustmemt(Interface);
		}*/
	}

	__forceinline bool IsNetReady(UNetConnection* Connection, bool bSaturate) {
		//bool (*IsNetReady)(UNetConnection*, bool) = decltype(IsNetReady)(OFFSET);
		//return IsNetReady(Connection, bSaturate);
		return true;
	}

	__forceinline bool IsNetReady(UChannel* Channel, bool bSaturate) {
		return IsNetReady(Channel->Connection, bSaturate);
	}

	__forceinline UActorChannel* FindChannel(AActor* Actor, UNetConnection* Connection) {
		for (UChannel* Chan : Connection->OpenChannels) {
			if (!Chan || !Chan->IsA(UActorChannel::StaticClass()))
				continue;

			UActorChannel* ActorChan = (UActorChannel*)Chan;
			if (ActorChan->Actor == Actor)
				return ActorChan;
		}
		return nullptr;
	}

	__forceinline UActorChannel* CreateActorChannel(UNetConnection* Conn, AActor* Actor) {
		using CreateChannelByNameFn = UChannel * (*)(UNetConnection*, FName*, EChannelCreateFlags, int32_t);
		using SetChannelActorFn = int64(*)(UActorChannel*, AActor*);

		static CreateChannelByNameFn CreateChannelByName = (CreateChannelByNameFn)(ImageBase + 0x16fda6c);
		static SetChannelActorFn SetChannelActor = (SetChannelActorFn)(ImageBase + 0x1273ec0);

		static FName ActorChannelName = FName(298);
		UActorChannel* Channel = (UActorChannel*)CreateChannelByName(Conn, &ActorChannelName, EChannelCreateFlags::OpenedLocally, -1);
		if (Channel) {
			SetChannelActor(Channel, Actor);
			return Channel;
		}
		return nullptr;
	}

	__forceinline bool ReplicateActorIfReady(UNetDriver* Driver, UNetConnection* Conn, UActorChannel* Channel, FNetworkObjectInfo* ActorInfo) {
		using ReplicateActorFn = int64(*)(UActorChannel*);
		static ReplicateActorFn ReplicateActor = (ReplicateActorFn)(ImageBase + 0x838c068);
		if (!Channel || !ReplicateActor || !IsNetReady(Channel, false))
			return false;
		if (ReplicateActor(Channel)) {
			ActorInfo->LastNetReplicateTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			return true;
		}
		return false;
	}

	bool IsConnectionInDormantSet(const TSet<TWeakObjectPtr<UNetConnection>>& Set, UNetConnection* Conn)
	{
		for (const auto& WeakConn : Set)
		{
			if (WeakConn.Get() == Conn)
				return true;
		}
		return false;
	}

	int32 ServerReplicateActors(UNetDriver* Driver, float DeltaTime) {
		if (!Driver || !Driver->World) {
			if (!Driver) {
				Log("NetDriver does not exist!");
			}
			else {
				Log("NetDriver World does not exist!");
			}
		}

		if (Driver->ClientConnections.Num() == 0)
		{
			return 0;
		}

		GetReplicationFrame(Driver)++;

		int NumConns = PrepConns(Driver);
		if (NumConns == 0) return 0;

		BuildConsiderList(Driver, DeltaTime);

		for (UNetConnection* Conn : Driver->ClientConnections) {
			if (!Conn || !Conn->ViewTarget) continue;

			if (Conn->PlayerController)
				SendClientAdjustment(Conn->PlayerController);
			for (auto& Child : Conn->Children)
				if (Child && Child->PlayerController)
					SendClientAdjustment(Child->PlayerController);

			TArray<FNetViewer> Viewers;
			Viewers.Add(ConstructNetViewer(Conn));

			for (FNetworkObjectInfo* ActorInfo : ConsiderList) {
				if (!ActorInfo || !ActorInfo->Actor) continue;
				AActor* Actor = ActorInfo->Actor;

				if (IsConnectionInDormantSet(ActorInfo->DormantConnections, Conn))
					continue;

				if (!IsActorRelevantToConnection(Actor, Viewers))
					continue;
				if (!IsLevelInitializedForActor(Driver, Actor, Conn))
					continue;

				UActorChannel* Channel = FindChannel(Actor, Conn);
				if (!Channel && IsNetReady(Conn, false)) {
					Channel = CreateActorChannel(Conn, Actor);
				}

				if (Channel) {
					ReplicateActorIfReady(Driver, Conn, Channel, ActorInfo);
				}
			}
		}

		ConsiderList.Free();

		return 0;
	}
}