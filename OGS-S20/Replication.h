#pragma once
#include "framework.h"
#include "Globals.h"

namespace Replication {
	struct FNetworkObjectInfo
	{
		/** Pointer to the replicated actor. */
		AActor* Actor;

		/** WeakPtr to actor. This is cached here to prevent constantly constructing one when needed for (things like) keys in TMaps/TSets */
		TWeakObjectPtr<AActor> WeakActor;

		/** Next time to consider replicating the actor. Based on FPlatformTime::Seconds(). */
		double NextUpdateTime;

		/** Last absolute time in seconds since actor actually sent something during replication */
		double LastNetReplicateTime;

		/** Optimal delta between replication updates based on how frequently actor properties are actually changing */
		float OptimalNetUpdateDelta;

		/** Last time this actor was updated for replication via NextUpdateTime
		* @warning: internal net driver time, not related to WorldSettings.TimeSeconds */
		double LastNetUpdateTimestamp;

		/** List of connections that this actor is dormant on */
		TSet<TWeakObjectPtr<class UNetConnection>> DormantConnections;

		/** A list of connections that this actor has recently been dormant on, but the actor doesn't have a channel open yet.
		*  These need to be differentiated from actors that the client doesn't know about, but there's no explicit list for just those actors.
		*  (this list will be very transient, with connections being moved off the DormantConnections list, onto this list, and then off once the actor has a channel again)
		*/
		TSet<TWeakObjectPtr<class UNetConnection>> RecentlyDormantConnections;

		/** Is this object still pending a full net update due to clients that weren't able to replicate the actor at the time of LastNetUpdateTime */
		uint8 bPendingNetUpdate : 1;

		/** Should this object be considered for replay checkpoint writes */
		uint8 bDirtyForReplay : 1;

		/** Should channel swap roles while calling ReplicateActor */
		uint8 bSwapRolesOnReplicate : 1;

		/** Force this object to be considered relevant for at least one update */
		uint32 ForceRelevantFrame = 0;

		FNetworkObjectInfo()
			: Actor(nullptr)
			, NextUpdateTime(0.0)
			, LastNetReplicateTime(0.0)
			, OptimalNetUpdateDelta(0.0f)
			, LastNetUpdateTimestamp(0.0)
			, bPendingNetUpdate(false)
			, bDirtyForReplay(false)
			, bSwapRolesOnReplicate(false) {
		}

		FNetworkObjectInfo(AActor* InActor)
			: Actor(InActor)
			, WeakActor(InActor)
			, NextUpdateTime(0.0)
			, LastNetReplicateTime(0.0)
			, OptimalNetUpdateDelta(0.0f)
			, LastNetUpdateTimestamp(0.0)
			, bPendingNetUpdate(false)
			, bDirtyForReplay(false)
			, bSwapRolesOnReplicate(false) {
		}
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

		inline void Remove(AActor* const Actor) // mannn idk, there is no children of TSet that allows us to remove an object from the set
		{
			if (Actor == nullptr)
			{
				return; //i come back to you when i finish shit
			}
		}
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

	struct FActorPriority
	{
		int32 Priority;
		FNetworkObjectInfo* ActorInfo;
		UActorChannel* Channel;

		FActorPriority()
			: Priority(0), ActorInfo(nullptr), Channel(nullptr) {
		}

		FActorPriority(UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo)
			: Priority(0), ActorInfo(InActorInfo), Channel(InChannel) {
		}
	};

	struct FInt32Interval
	{
		int32 Min;
		int32 Max;

		FInt32Interval(int32 InMin, int32 InMax)
			: Min(InMin), Max(InMax)
		{
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

	bool IsRelevancyOwnerFor(AActor* Actor, const AActor* ActorOwner)
	{
		return (ActorOwner == Actor);
	}

	int32 ServerReplicateActors_PrepConnections(UNetDriver* Driver, float DeltaTime)
	{
		if (!Driver) return 0;

		int32 NumClientsToTick = Driver->ClientConnections.Num();
		bool bFoundReadyConnection = false;

		for (UNetConnection* Conn : Driver->ClientConnections) {
			AActor* OwningActor = Conn->OwningActor;

			if (OwningActor && (GetElapsedTime(Driver) - Conn->LastReceiveTime < 1.5f)) {
				bFoundReadyConnection = true;

				AActor* DesiredViewTarget = OwningActor;

				if (Conn->PlayerController) {
					if (AActor* ViewTarget = Conn->PlayerController->GetViewTarget()) {
						DesiredViewTarget = ViewTarget;
					}
				}

				Conn->ViewTarget = DesiredViewTarget;

				for (UNetConnection* Child : Conn->Children) {
					AActor* DesiredChildViewTarget = Child->OwningActor;

					if (Child->PlayerController) {
						if (AActor* ChildViewTarget = Child->PlayerController->GetViewTarget()) {
							DesiredChildViewTarget = ChildViewTarget;
						}
					}

					Child->ViewTarget = DesiredChildViewTarget;
				}
			}
			else {
				Conn->ViewTarget = nullptr;
				for (UNetConnection* Child : Conn->Children) {
					Child->ViewTarget = nullptr;
				}
			}
		}

		/*for (auto& Conn : Driver->ClientConnections)
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
		}*/

		return bFoundReadyConnection ? NumClientsToTick : 0;
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

	bool IsNetStartupActor(AActor* Actor)
	{
		return Actor->bNetStartup || (!Actor->bActorSeamlessTraveled && Actor->bNetLoadOnClient);
	}

	bool IsDormInitialStartupActor(AActor* Actor)
	{
		return Actor && IsNetStartupActor(Actor) && (Actor->NetDormancy == ENetDormancy::DORM_Initial);
	}

	void RemoveNetworkActor(UNetDriver* Driver, AActor* Actor)
	{
		if (!Driver || !Actor)
			return;

		GetNetworkObjectList(Driver).Remove(Actor);
	}

	void ServerReplicateActors_BuildConsiderList(UNetDriver* Driver, TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime) {
		//Log("ServerReplicateActors_BuildConsiderList, Building ConsiderList at WorldTime : " + std::to_string(UGameplayStatics::GetTimeSeconds(UWorld::GetWorld())) + " ServerTickTime : " + std::to_string(ServerTickTime));

		if (!&OutConsiderList) {
			Log("NetworkObjectList is null!");
			return;
		}

		int32 NumInitiallyDormant = 0;

		TArray<AActor*> ActorsToRemove;

		for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList(Driver).ActiveNetworkObjects)
		{
			if (!ObjectInfo.Get())
				continue;

			FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();
			
			if (!ActorInfo->bPendingNetUpdate && UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) <= ActorInfo->NextUpdateTime)
				continue;

			AActor* Actor = ActorInfo->Actor;

			if (Actor->bActorIsBeingDestroyed) //Actor->IsPendingKillPending
			{
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (Actor->GetRemoteRole() == ENetRole::ROLE_None)
			{
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (Actor->NetDriverName != Driver->NetDriverName) //Actor->GetNetDriverName() != NetDriverName
				continue;

			ULevel* Level = Actor->GetLevel();
			if (Level->OwningWorld && (Level == Level->OwningWorld->CurrentLevelPendingVisibility || Level == Level->OwningWorld->CurrentLevelPendingInvisibility)) //Level->HasVisibilityChangeRequestPending
				continue;

			if (IsDormInitialStartupActor(Actor))
			{
				NumInitiallyDormant++;
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (ActorInfo->LastNetReplicateTime == 0)
			{
				ActorInfo->LastNetReplicateTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
				ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency;
			}

			const float ScaleDownStartTime = 2.0f;
			const float ScaleDownTimeRange = 5.0f;

			const float LastReplicateDelta = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());

			if (LastReplicateDelta > ScaleDownStartTime)
			{
				if (Actor->MinNetUpdateFrequency == 0.0f)
					Actor->MinNetUpdateFrequency = 2.0f;

				const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
				const float MaxOptimalDelta = UKismetMathLibrary::GetDefaultObj()->Max(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta);

				const float Alpha = UKismetMathLibrary::GetDefaultObj()->Clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
				ActorInfo->OptimalNetUpdateDelta = UKismetMathLibrary::GetDefaultObj()->Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
			}

			if (!ActorInfo->bPendingNetUpdate)
			{
				const float NextUpdateDelta = ActorInfo->OptimalNetUpdateDelta;

				float RandDelay = 0.0f;

				ActorInfo->NextUpdateTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) + RandDelay * ServerTickTime + NextUpdateDelta;

				ActorInfo->LastNetUpdateTimestamp = GetElapsedTime(Driver);
			}

			ActorInfo->bPendingNetUpdate = false;

			OutConsiderList.Add(ActorInfo);

			static void (*CallPreReplication)(AActor*, UNetDriver * NetDriver) = decltype(CallPreReplication)(ImageBase + 0x82A7038);
			CallPreReplication(Actor, Driver);
		}

		/* Work on you soon enough
		for (AActor* Actor : ActorsToRemove)
		{
			RemoveNetworkActor(Driver, Actor);
		}*/

		ActorsToRemove.Free();
	}

	__forceinline bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
	{
		bool (*IsNetRelevantFor)(const AActor*, const AActor*, const AActor*, const FVector&) = decltype(IsNetRelevantFor)(Actor->VTable[0x9a]);

		for (auto& Viewer : ConnectionViewers)
		{
			if (IsNetRelevantFor(Actor, Viewer.InViewer, Viewer.ViewTarget, Viewer.ViewLocation))
			{
				return true;
			}
		}

		return false;
		//return true;
	}

	static UNetConnection* IsActorOwnedByAndRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
	{
		const AActor* ActorOwner = Actor->GetOwner();

		bOutHasNullViewTarget = false;

		for (int i = 0; i < ConnectionViewers.Num(); i++)
		{
			UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

			if (ViewerConnection->ViewTarget == nullptr)
			{
				bOutHasNullViewTarget = true;
			}

			if (ActorOwner == ViewerConnection->PlayerController ||
				(ViewerConnection->PlayerController && ActorOwner == ViewerConnection->PlayerController->Pawn) ||
				(ViewerConnection->ViewTarget && IsRelevancyOwnerFor(ViewerConnection->ViewTarget, ActorOwner)))
			{
				return ViewerConnection;
			}
		}

		return nullptr;
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
		bool (*ClientHasInitializedLevelFor)(const UNetConnection*, const AActor*) = decltype(ClientHasInitializedLevelFor)(ImageBase + 0x8473B58);

		const bool bCorrectWorld = NetDriver->WorldPackage != nullptr && (GetClientWorldPackageName(InConnection) == NetDriver->WorldPackage->Name) && ClientHasInitializedLevelFor(InConnection, InActor);
		const bool bIsConnectionPC = (InActor == InConnection->PlayerController);
		return bCorrectWorld || bIsConnectionPC;
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
	}

	__forceinline bool IsNetReady(UNetConnection* Connection, bool bSaturate) {
		bool (*IsNetReady)(UNetConnection*, bool) = decltype(IsNetReady)(ImageBase + 0x8479048);
		return IsNetReady(Connection, bSaturate);
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
		}//thinking

		return nullptr;
	}

	using ReplicateActorFn = int64(*)(UActorChannel*);
	static ReplicateActorFn ReplicateActor = (ReplicateActorFn)(ImageBase + 0x838c068);

	__forceinline bool ReplicateActorIfReady(UNetDriver* Driver, UNetConnection* Conn, UActorChannel* Channel, FNetworkObjectInfo* ActorInfo) {
		if (!Channel || !ReplicateActor || !IsNetReady(Channel, false))
			return false;

		bool bReplicated = ReplicateActor(Channel);
		if (bReplicated) {
			ActorInfo->LastNetReplicateTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			ActorInfo->Actor->ForceNetUpdate();
			Channel->Actor->ForceNetUpdate();

			if (ActorInfo->Actor->bReplicateMovement) {
				ActorInfo->Actor->OnRep_ReplicateMovement();
				ActorInfo->Actor->OnRep_ReplicatedMovement();
			}
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

	enum class EChannelCloseReason : uint8_t
	{
		Destroyed,
		Dormancy,
		LevelUnloaded,
		Relevancy,
		TearOff,
		/* reserved */
		MAX = 15		// this value is used for serialization, modifying it may require a network version change
	};

	void ActorChannelClose(UActorChannel* Channel, EChannelCloseReason CloseReason)
	{
		void (*ActorChannelCloseOG)(UActorChannel * Channel, EChannelCloseReason CloseReason) = decltype(ActorChannelCloseOG)(ImageBase + 0x8381470);
		return ActorChannelCloseOG(Channel, CloseReason);
	}

	int32 ServerReplicateActors_PrioritizeActors(
		UNetDriver* Driver,
		UNetConnection* Connection,
		const TArray<FNetViewer>& ConnectionViewers,
		const TArray<FNetworkObjectInfo*>& ConsiderList,
		FActorPriority*& OutPriorityList,
		FActorPriority**& OutPriorityActors)
	{
		GetNetTag(Driver)++;

		for (int32 j = 0; j < Connection->SentTemporaries.Num(); j++)
			Connection->SentTemporaries[j]->NetTag = GetNetTag(Driver);

		int32 FinalSortedCount = 0;
		int32 DeletedCount = 0; //i think??

		//TWeakObjectPtr<UNetConnection> WeakConnection(Connection);

		const int32 MaxSortedActors = ConsiderList.Num() + GetDestroyedStartupOrDormantActors(Driver).Num();
		if (MaxSortedActors > 0)
		{
			for (FNetworkObjectInfo* ActorInfo : ConsiderList)
			{
				AActor* Actor = ActorInfo->Actor;
				
				UActorChannel* Channel = FindChannel(Actor, Connection);

				if (!Channel)
				{
					if (!IsLevelInitializedForActor(Driver, Actor, Connection))
						continue;

					if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
						continue;
				}

				UNetConnection* PriorityConnection = Connection;

				if (Actor->bOnlyRelevantToOwner)
				{
					bool bHasNullViewTarget = false;

					PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

					if (PriorityConnection == nullptr)
					{
						if (!bHasNullViewTarget && Channel != NULL)
							ActorChannelClose(Channel, EChannelCloseReason::Relevancy);
					}

					continue;
				}

				if (Actor->NetTag != GetNetTag(Driver))
				{
					Actor->NetTag = GetNetTag(Driver);
					OutPriorityList[FinalSortedCount] = FActorPriority(Channel, ActorInfo);
					OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
					FinalSortedCount++;
				}
			}
		}

		return FinalSortedCount;
	}

	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetDriver* Driver, UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
	{
		int32 FinalRelevantCount = 0;

		if (IsNetReady(Connection, false)) //false = 0
			return 0;

		for (int32 j = 0; j < FinalSortedCount; j++)
		{
			FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;

			if (ActorInfo == NULL)
				continue;

			UActorChannel* Channel = PriorityActors[j]->Channel;

			if (!Channel || Channel->Actor)
			{
				AActor* Actor = ActorInfo->Actor;
				bool bIsRelevant = false;

				const bool bLevelInitializedForActor = IsLevelInitializedForActor(Driver, Actor, Connection);

				if (bLevelInitializedForActor)
				{
					if (!Actor->bTearOff && !Channel)
					{
						if (IsActorRelevantToConnection(Actor, ConnectionViewers))
						{
							bIsRelevant = true;
						}
					}
				}

				const bool bIsRecentlyRelevant = bIsRelevant;

				if (bIsRecentlyRelevant)
				{
					FinalRelevantCount++;

					if (Channel == NULL)
					{
						if (bLevelInitializedForActor)
						{
							Channel = CreateActorChannel(Connection, Actor);
						}
					}
					else if (Actor->NetUpdateFrequency < 1.0f)
					{
						ActorInfo->NextUpdateTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) + 0.2f * UKismetMathLibrary::GetDefaultObj()->RandomFloat();
					}
				}

				if (Channel)
				{
					if (IsNetReady(Channel, false))
					{
						if (ReplicateActor(Channel))
						{
							// Calculate min delta (max rate actor will upate), and max delta (slowest rate actor will update)
							const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
							const float MaxOptimalDelta = UKismetMathLibrary::GetDefaultObj()->Max(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta);
							const float DeltaBetweenReplications = (UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) - ActorInfo->LastNetReplicateTime);

							// Choose an optimal time, we choose 70% of the actual rate to allow frequency to go up if needed
							ActorInfo->OptimalNetUpdateDelta = UKismetMathLibrary::GetDefaultObj()->Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
							ActorInfo->LastNetReplicateTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
						}
						OutUpdated++;
					}
					else
					{
						Actor->ForceNetUpdate();
					}

					if (!IsNetReady(Connection, false))
						return j;
				}

				if ((!bIsRecentlyRelevant || Actor->bTearOff) && Channel != NULL)
				{
					if (!bLevelInitializedForActor || !IsNetStartupActor(Actor))
						ActorChannelClose(Channel, Actor->bTearOff ? EChannelCloseReason::TearOff : EChannelCloseReason::Relevancy);
				}
			}
		}

		return FinalRelevantCount;
	}

	void ServerReplicateActors_MarkRelevantActors(
		UNetConnection* Connection,
		const TArray<FNetViewer>& ConnectionViewers,
		int32 StartActorIndex, int32 EndActorIndex,
		FActorPriority** PriorityActors)
	{
		for (int32 k = StartActorIndex; k < EndActorIndex; k++)
		{
			if (!PriorityActors[k]->ActorInfo)
			{
				continue;
			}

			AActor* Actor = PriorityActors[k]->ActorInfo->Actor;

			UActorChannel* Channel = PriorityActors[k]->Channel;

			if (Channel != NULL)
			{
				PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
			}
			else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
			{
				PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
			}
		}
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
			return 0;

		GetReplicationFrame(Driver)++;

		int32 Updated = 0;

		const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(Driver, DeltaTime);
		if (NumClientsToTick == 0)
			return 0;

		float ServerTickTime = Globals::MaxTickRate;
		if (ServerTickTime == 0.f)
		{
			ServerTickTime = DeltaTime;
		}
		else
		{
			ServerTickTime = 1.f / ServerTickTime;
		}

		TArray<FNetworkObjectInfo*> ConsiderList;
		ConsiderList.Reserve(GetNetworkObjectList(Driver).ActiveNetworkObjects.Num()); //Driver->GetNetworkObjectList().GetActiveObjects().Num()

		ServerReplicateActors_BuildConsiderList(Driver, ConsiderList, ServerTickTime);

		for (int32 i = 0; i < Driver->ClientConnections.Num(); i++)
		{
			UNetConnection* Connection = Driver->ClientConnections[i];
			if (!Connection) //check(Connection);
				continue;

			if (i >= NumClientsToTick)
				continue;

			if (!Connection->ViewTarget)
			{
				TArray<FNetViewer>& ConnectionViewers = AWorldSettings::GetDefaultObj()->ReplicationViewers;

				ConstructNetViewer(Connection);
				for (int32 ViewerIndex = 0; ViewerIndex < Connection->Children.Num(); ViewerIndex++)
				{
					if (Connection->Children[ViewerIndex]->ViewTarget != NULL)
						ConstructNetViewer(Connection->Children[ViewerIndex]);
				}

				if (Connection->PlayerController)
					SendClientAdjustment(Connection->PlayerController);

				for (int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++)
				{
					if (Connection->Children[ChildIdx]->PlayerController != NULL)
						SendClientAdjustment(Connection->Children[ChildIdx]->PlayerController);
				}

				FActorPriority* PriorityList = NULL;
				FActorPriority** PriorityActors = NULL;

				const int32 FinalSortCount = ServerReplicateActors_PrioritizeActors(Driver, Connection, ConnectionViewers, ConsiderList, PriorityList, PriorityActors);
				const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(Driver, Connection, ConnectionViewers, PriorityActors, FinalSortCount, Updated);

				for (int32 k = LastProcessedActor; k < FinalSortCount; k++)
				{
					if (!PriorityActors[k]->ActorInfo)
						continue;

					AActor* Actor = PriorityActors[k]->ActorInfo->Actor;
					UActorChannel* Channel = PriorityActors[k]->Channel;

					if (Channel != NULL)
					{
						PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
					}
					else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
					{
						PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
					}
				}

				if (NumClientsToTick < Driver->ClientConnections.Num())
				{
					int32 NumConnectionsToMove = NumClientsToTick;
					while (NumConnectionsToMove > 0)
					{
						UNetConnection* Connection = Driver->ClientConnections[0];
						Driver->ClientConnections.Remove(0);
						Driver->ClientConnections.Add(Connection);
						NumConnectionsToMove--;
					}
				}
			}
		}

		return Updated;
	}
}