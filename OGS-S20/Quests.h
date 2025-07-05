#pragma once
#include "framework.h"

namespace Quests {
	void GiveAccolade(AFortPlayerControllerAthena* PC, UFortAccoladeItemDefinition* Def, UFortQuestItemDefinition* QuestDef = nullptr)
	{
		if (!PC || !Def) return;

		FAthenaAccolades Accolade{};
		Accolade.AccoladeDef = Def;
		Accolade.Count = 1;
		std::string DefName = Def->GetName();
		Accolade.TemplateId = std::wstring(DefName.begin(), DefName.end()).c_str();

		auto ID = UKismetSystemLibrary::GetDefaultObj()->GetPrimaryAssetIdFromObject(Def);

		FXPEventInfo EventInfo{};
		EventInfo.Accolade = ID;
		EventInfo.EventName = Def->Name;
		EventInfo.EventXpValue = Def->GetAccoladeXpValue();
		EventInfo.Priority = Def->Priority;
		if (QuestDef) {
			EventInfo.QuestDef = QuestDef;
		}
		EventInfo.SimulatedText = Def->GetShortDescription();
		EventInfo.RestedValuePortion = EventInfo.EventXpValue;
		EventInfo.RestedXPRemaining = EventInfo.EventXpValue;
		EventInfo.SeasonBoostValuePortion = 20;
		EventInfo.TotalXpEarnedInMatch = EventInfo.EventXpValue + PC->XPComponent->TotalXpEarned;

		PC->XPComponent->MedalBonusXP += 1250;
		PC->XPComponent->MatchXp += EventInfo.EventXpValue;
		PC->XPComponent->TotalXpEarned += EventInfo.EventXpValue + 1250;

		PC->XPComponent->PlayerAccolades.Add(Accolade);
		PC->XPComponent->MedalsEarned.Add(Def);

		PC->XPComponent->ClientMedalsRecived(PC->XPComponent->PlayerAccolades);
		PC->XPComponent->OnXpEvent(EventInfo);
	}
}