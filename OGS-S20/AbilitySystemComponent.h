#pragma once
#include "framework.h"

namespace AbilitySystemComponent {
	inline void GiveAbility(UFortAbilitySystemComponent* AbilitySystemComponent, UGameplayAbility* Ability) {
		FGameplayAbilitySpec Spec{};
		AbilitySpecConstructor(&Spec, Ability, 1, -1, nullptr);
		GiveAbilityOG(AbilitySystemComponent, &Spec.Handle, Spec);
		//Log("Given Ability: " + AbilitySet->GameplayAbilities[i].Get()->GetName());
	}

	inline void GrantAbilitySet(AFortPlayerController* PC, UFortAbilitySet* AbilitySet) {
		auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

		TScriptInterface<IAbilitySystemInterface> Interface{};
		Interface.ObjectPointer = PlayerState;
		Interface.InterfacePointer = GetInterface<IAbilitySystemInterface>(PlayerState);

		if (PlayerState && AbilitySet)
		{
			for (size_t i = 0; i < AbilitySet->GameplayAbilities.Num(); i++)
			{
				GiveAbility(PlayerState->AbilitySystemComponent, (UGameplayAbility*)AbilitySet->GameplayAbilities[i].Get()->DefaultObject);
			}

			for (auto& GameplayEffect : AbilitySet->PassiveGameplayEffects) {
				PlayerState->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GameplayEffect.GameplayEffect.Get(), GameplayEffect.Level, PlayerState->AbilitySystemComponent->MakeEffectContext());
			}
			UFortKismetLibrary::EquipFortAbilitySet(Interface, AbilitySet, nullptr);
		}
	}

	inline void InitAbilitiesForPlayer(AFortPlayerController* PC)
	{
		auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		if (!PlayerState) {
			return;
		}

		static UFortAbilitySet* AbilitySet = StaticLoadObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
		static UFortAbilitySet* TacticalSprintAbilitySet = StaticLoadObject<UFortAbilitySet>("/TacticalSprint/Gameplay/AS_TacticalSprint.AS_TacticalSprint");
		static UFortAbilitySet* AscenderAbilitySet = StaticLoadObject<UFortAbilitySet>("/Ascender/Gameplay/Ascender/AS_Ascender.AS_Ascender");

		GrantAbilitySet(PC, AbilitySet);
		GrantAbilitySet(PC, TacticalSprintAbilitySet);
		GrantAbilitySet(PC, AscenderAbilitySet);

		static UBlueprintGeneratedClass* GrantTacticalSprintAbility = StaticLoadObject<UBlueprintGeneratedClass>("/TacticalSprint/Gameplay/GA_Athena_GrantTacticalSprint.GA_Athena_GrantTacticalSprint_C");
		GiveAbility(PlayerState->AbilitySystemComponent, (UGameplayAbility*)GrantTacticalSprintAbility->DefaultObject);
	}

	inline FGameplayAbilitySpec* FindAbilitySpecFromHandle(UFortAbilitySystemComponentAthena* ASC, FGameplayAbilitySpecHandle& Handle)
	{
		for (size_t i = 0; i < ASC->ActivatableAbilities.Items.Num(); i++)
		{
			if (ASC->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle)
				return &ASC->ActivatableAbilities.Items[i];
		}
		return nullptr;
	}


	FGameplayAbilitySpec* FindAbilitySpec(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle) {

		for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->ActivatableAbilities.Items) {
			if (Spec.Handle.Handle == Handle.Handle) {
				if (!Spec.PendingRemove) {
					return &Spec;
				}
			}
		}

		return nullptr;
	}

	void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const struct FPredictionKey& InPredictionKey, FGameplayEventData* TriggerEventData)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpec(AbilitySystemComponent, Handle);
		if (!Spec) {
			return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, InPredictionKey.Current);
		}
		UGameplayAbility* AbilityToActivate = Spec->Ability;

		if (!AbilityToActivate)
		{
			return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, InPredictionKey.Current);
		}

		UGameplayAbility* InstancedAbility = nullptr;
		Spec->InputPressed = true;

		if (InternalTryActivateAbility(AbilitySystemComponent, Handle, InPredictionKey, &InstancedAbility, nullptr, TriggerEventData)) {}
		else {

			AbilitySystemComponent->ClientActivateAbilityFailed(Handle, InPredictionKey.Current);
			Spec->InputPressed = false;

			AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
		}
	}

	void Hook()
	{
		HookVTable(UAbilitySystemComponent::GetDefaultObj(), 0x10B, InternalServerTryActivateAbility, nullptr);
		HookVTable(UFortAbilitySystemComponent::GetDefaultObj(), 0x10B, InternalServerTryActivateAbility, nullptr);
		HookVTable(UFortAbilitySystemComponentAthena::GetDefaultObj(), 0x10B, InternalServerTryActivateAbility, nullptr);

		Log("Abilities Hooked!");
	}
}