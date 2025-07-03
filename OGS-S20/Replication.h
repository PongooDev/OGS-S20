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
				return;
			}

			auto RemoveFromSet = [&](FNetworkObjectSet& Set)
				{
					for (int32 i = 0; i < Set.Elements.NumAllocated();)
					{
						if (!Set.Elements.IsValidIndex(i))
						{
							++i;
							continue;
						}

						TSharedPtr<FNetworkObjectInfo>& InfoPtr = Set.Elements[i].Value;

						if (InfoPtr.Get() && InfoPtr.Get()->Actor == Actor)
						{
							Set.Elements[i] = Set.Elements[Set.Elements.NumAllocated() - 1];
							Set.Elements.Data.Remove(Set.Elements.NumAllocated() - 1);
						}
						else
						{
							++i;
						}
					}
				};

			RemoveFromSet(ActiveNetworkObjects);
			RemoveFromSet(AllNetworkObjects);
			RemoveFromSet(ObjectsDormantOnAllConnections);
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

	bool IsDormInitialStartupActor(AActor* Actor)
	{
		return Actor && (Actor->NetDormancy == ENetDormancy::DORM_Initial);
	}

	void RemoveNetworkActor(UNetDriver* Driver, AActor* Actor)
	{
		if (!Driver || !Actor)
			return;

		GetNetworkObjectList(Driver).Remove(Actor);
	}

	void ServerReplicateActors_BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, UNetDriver* Driver, const float ServerTickTime) {
		//Log("ServerReplicateActors_BuildConsiderList, Building ConsiderList at WorldTime : " + std::to_string(UGameplayStatics::GetTimeSeconds(UWorld::GetWorld())) + " ServerTickTime : " + std::to_string(ServerTickTime));

		if (!&OutConsiderList) {
			Log("NetworkObjectList is null!");
			return;
		}

		int32 NumInitiallyDormant = 0;

		TArray<AActor*> ActorsToRemove;

		for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList(Driver).ActiveNetworkObjects)
		{
			FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();
			if (!ActorInfo) {
				continue;
			}

			if (!ActorInfo->bPendingNetUpdate && UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) <= ActorInfo->NextUpdateTime)
			{
				continue;
			}

			AActor* Actor = ActorInfo->Actor;
			if (!Actor) {
				continue;
			}

			if (Actor->bActorIsBeingDestroyed)
			{
				//Log("Actor " + Actor->GetName() + " was found in the NetworkObjectList, but is PendingKillPending");
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (Actor->RemoteRole == ENetRole::ROLE_None)
			{
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (Actor->NetDriverName != Driver->NetDriverName)
			{
				//Log("Actor " + Actor->GetName() + " in wrong network actors list! (Has net driver '" + Actor->NetDriverName.ToString() + "', expected '" + Driver->NetDriverName.ToString() + "')");
				continue;
			}

			if (IsDormInitialStartupActor(Actor))
			{
				NumInitiallyDormant++;
				ActorsToRemove.Add(Actor);
				continue;
			}

			if (ActorInfo->LastNetReplicateTime == 0)
			{
				ActorInfo->LastNetReplicateTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
				ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency;
			}

			const float ScaleDownStartTime = 2.0f;
			const float ScaleDownTimeRange = 5.0f;

			const float LastReplicateDelta = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) - ActorInfo->LastNetReplicateTime;

			if (LastReplicateDelta > ScaleDownStartTime)
			{
				if (Actor->MinNetUpdateFrequency == 0.0f)
				{
					Actor->MinNetUpdateFrequency = 2.0f;
				}

				const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
				const float MaxOptimalDelta = UKismetMathLibrary::GetDefaultObj()->Max(1.0f / Actor->NetUpdateFrequency, MinOptimalDelta);

				const float Alpha = UKismetMathLibrary::GetDefaultObj()->Clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
				ActorInfo->OptimalNetUpdateDelta = UKismetMathLibrary::GetDefaultObj()->Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
			}

			if (!ActorInfo->bPendingNetUpdate)
			{
				//Log("actor " + Actor->GetName() + " requesting new net update, time: " + std::to_string(UGameplayStatics::GetTimeSeconds(UWorld::GetWorld())));

				const float NextUpdateDelta = ActorInfo->OptimalNetUpdateDelta;

				float RandDelay = 0.0f;

				ActorInfo->NextUpdateTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + RandDelay + NextUpdateDelta;

				ActorInfo->LastNetUpdateTimestamp = (float)GetElapsedTime(Driver);
			}

			ActorInfo->bPendingNetUpdate = false;

			OutConsiderList.Add(ActorInfo);

			Actor->bCallPreReplication = true;
			static void (*CallPreReplication)(AActor*, UNetDriver * NetDriver) = decltype(CallPreReplication)(ImageBase + 0x82A7038);
			CallPreReplication(Actor, Driver); // Doesent exist (maybe version specific) (Exist now 6/24/25)
		}

		for (AActor* Actor : ActorsToRemove)
		{
			RemoveNetworkActor(Driver, Actor);
		}

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
		return true;
		/*bool (*ClientHasInitializedLevelFor)(const UNetConnection*, const AActor*) = decltype(ClientHasInitializedLevelFor)(ImageBase + 0x8473B58);
		return ClientHasInitializedLevelFor(InConnection, InActor);*/

		/*const bool bCorrectWorld = NetDriver->WorldPackage != nullptr && (GetClientWorldPackageName(InConnection) == NetDriver->WorldPackage->Name) && ClientHasInitializedLevelFor(InConnection, InActor);
		const bool bIsConnectionPC = (InActor == InConnection->PlayerController);
		return bCorrectWorld || bIsConnectionPC;*/
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
		}
		return nullptr;
	}

	__forceinline bool ReplicateActorIfReady(UNetDriver* Driver, UNetConnection* Conn, UActorChannel* Channel, FNetworkObjectInfo* ActorInfo) {
		using ReplicateActorFn = int64(*)(UActorChannel*);
		static ReplicateActorFn ReplicateActor = (ReplicateActorFn)(ImageBase + 0x838c068);
		if (!Channel || !ReplicateActor || !IsNetReady(Channel, false))
			return false;

		bool bReplicated = ReplicateActor(Channel);
		if (bReplicated) {
			ActorInfo->LastNetReplicateTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			//ActorInfo->Actor->ForceNetUpdate();
			//Channel->Actor->ForceNetUpdate();

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

	int32 ServerReplicateActors_PrioritizeActors(
		UNetDriver* Driver,
		UNetConnection* Connection,
		const TArray<FNetViewer>& ConnectionViewers,
		const TArray<FNetworkObjectInfo*>& ConsiderList,
		const bool bCPUSaturated,
		FActorPriority*& OutPriorityList,
		FActorPriority**& OutPriorityActors)
	{
		GetNetTag(Driver)++;

		for (int32 j = 0; j < Connection->SentTemporaries.Num(); j++)
		{
			Connection->SentTemporaries[j]->NetTag = GetNetTag(Driver);
		}

		int32 FinalSortedCount = 0;
		int32 DeletedCount = 0;

		const int32 MaxSortedActors = ConsiderList.Num();

		if (MaxSortedActors > 0)
		{
			OutPriorityList = new FActorPriority[MaxSortedActors];
			OutPriorityActors = new FActorPriority * [MaxSortedActors];

			bool bLowNetBandwidth = false;

			for (FNetworkObjectInfo* ActorInfo : ConsiderList)
			{
				AActor* Actor = ActorInfo->Actor;
				if (!Actor) continue;

				UActorChannel* Channel = FindChannel(Actor, Connection);

				if (!Channel && !IsActorRelevantToConnection(Actor, ConnectionViewers))
					continue;

				if (Actor->NetTag == GetNetTag(Driver))
					continue;

				Actor->NetTag = GetNetTag(Driver);

				FVector ViewLocation = ConnectionViewers[0].ViewLocation;
				float DistanceSquared = (Actor->K2_GetActorLocation() - ViewLocation).SizeSquared();

				int32 Priority = static_cast<int32>(UKismetMathLibrary::GetDefaultObj()->Clamp(1000000.0f - DistanceSquared, 0.0f, 1000000.0f));

				OutPriorityList[FinalSortedCount] = FActorPriority(Channel, ActorInfo);
				OutPriorityList[FinalSortedCount].Priority = Priority;
				OutPriorityActors[FinalSortedCount] = &OutPriorityList[FinalSortedCount];
				FinalSortedCount++;
			}

			std::sort(OutPriorityActors, OutPriorityActors + FinalSortedCount, [](FActorPriority* A, FActorPriority* B)
				{
					return A->Priority > B->Priority;
				});
		}

		return FinalSortedCount;
	}

	int32 ServerReplicateActors_ProcessPrioritizedActorsRange(
		UNetDriver* Driver,
		UNetConnection* Connection,
		const TArray<FNetViewer>& ConnectionViewers,
		FActorPriority** PriorityActors,
		int32 StartIndex,
		int32 Count,
		int32& OutUpdated,
		bool bIgnoreSaturation)
	{
		int32 FinalRelevantCount = 0;

		for (int32 j = StartIndex; j < (StartIndex + Count); j++)
		{
			FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;
			if (!ActorInfo || !ActorInfo->Actor) continue;

			AActor* Actor = ActorInfo->Actor;
			UActorChannel* Channel = FindChannel(Actor, Connection);

			if (!Channel && IsNetReady(Connection, false)) {
				Channel = CreateActorChannel(Connection, Actor);
			}

			if (Channel && ReplicateActorIfReady(Driver, Connection, Channel, ActorInfo)) {
				OutUpdated++;
				FinalRelevantCount++;
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
		{
			return 0;
		}

		GetReplicationFrame(Driver)++;

		int32 Updated = 0;

		const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(Driver, DeltaTime);
		if (NumClientsToTick == 0) return 0;

		AWorldSettings* WorldSettings = Driver->World->K2_GetWorldSettings();

		bool bCPUSaturated = false;
		float ServerTickTime = Globals::MaxTickRate;
		if (ServerTickTime == 0.f)
		{
			ServerTickTime = DeltaTime;
		}
		else
		{
			ServerTickTime = 1.f / ServerTickTime;
			bCPUSaturated = DeltaTime > 1.2f * ServerTickTime;
		}

		TArray<FNetworkObjectInfo*> ConsiderList;
		ConsiderList.Reserve(GetNetworkObjectList(Driver).ActiveNetworkObjects.Num());

		ServerReplicateActors_BuildConsiderList(ConsiderList, Driver, ServerTickTime);

		TArray<UNetConnection*> ConnectionsToClose;

		for (int32 i = 0; i < Driver->ClientConnections.Num(); i++)
		{
			UNetConnection* Connection = Driver->ClientConnections[i];
			if (!Connection) {
				// This should not happen at all, else its pretty bad!
				Log("Connection is nullptr!");
				continue;
			}

			// if this client shouldn't be ticked this frame
			if (i >= NumClientsToTick) {}
			else if (Connection->ViewTarget)
			{
				TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

				ConnectionViewers.Free();
				ConnectionViewers.Add(FNetViewer(Connection));
				for (int32 ViewerIndex = 0; ViewerIndex < Connection->Children.Num(); ViewerIndex++)
				{
					if (Connection->Children[ViewerIndex]->ViewTarget != NULL)
					{
						ConnectionViewers.Add(FNetViewer(Connection->Children[ViewerIndex]));
					}
				}

				if (Connection->PlayerController)
				{
					SendClientAdjustment(Connection->PlayerController);
				}

				for (int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++)
				{
					if (Connection->Children[ChildIdx]->PlayerController != NULL)
					{
						SendClientAdjustment(Connection->Children[ChildIdx]->PlayerController);
					}
				}

				FActorPriority* PriorityList = nullptr;
				FActorPriority** PriorityActors = nullptr;

				const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(Driver, Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

				const int32 StartIndex = 0;

				const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActorsRange(Driver, Connection, ConnectionViewers, PriorityActors, StartIndex, FinalSortedCount, Updated, false);

				ServerReplicateActors_MarkRelevantActors(Connection, ConnectionViewers, LastProcessedActor, FinalSortedCount, PriorityActors);

				ConnectionViewers.Free();
			}

			if (GetPendingCloseDueToReplicationFailure(Connection))
			{
				ConnectionsToClose.Add(Connection);
			}
		}

		// shuffle the list of connections if not all connections were ticked
		if (NumClientsToTick < Driver->ClientConnections.Num())
		{
			TArray<UNetConnection*> OriginalConnections = Driver->ClientConnections;
			Driver->ClientConnections.Clear();
			Driver->ClientConnections.Reserve(OriginalConnections.Num());

			for (int i = NumClientsToTick; i < OriginalConnections.Num(); i++) {
				Driver->ClientConnections.Add(OriginalConnections[i]);
			}
			for (int i = 0; i < NumClientsToTick; i++) {
				Driver->ClientConnections.Add(OriginalConnections[i]);
			}
		}

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

		return Updated;
	}
}