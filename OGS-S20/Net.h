#pragma once
#include "framework.h"

namespace Net {
	// https://docs.unrealengine.com/4.26/en-US/API/Runtime/Engine/Engine/ENetMode/
	enum ENetMode
	{
		NM_Standalone,
		NM_DedicatedServer,
		NM_ListenServer,
		NM_Client,
		NM_MAX,
	};

	constexpr ENetMode NetMode = ENetMode::NM_DedicatedServer;

	static ENetMode GetNetMode()
	{
		return NetMode;
	}

	void Hook() {
		//MH_CreateHook((LPVOID)(ImageBase + ), AActorGetNetMode, nullptr); i have yet to find this
		MH_CreateHook((LPVOID)(ImageBase + 0xf5d6f8), GetNetMode, nullptr);

		Log("Hooked Net!");
	}
}