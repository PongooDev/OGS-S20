#pragma once
#include "framework.h"
#include "BehaviorTreeSystem.h"

#include "BehaviorTreeDecorators.h"
#include "BehaviorTreeEvaluators.h"
#include "BehaviorTreeServices.h"
#include "BehaviorTreeTasks.h"

namespace Npcs {
	struct BT_NPC_Context : BTContext
	{
		class NpcBot* bot;
	};

	std::vector<class NpcBot*> NpcBots{};
	class NpcBot
	{
	public:
		// The behaviortree for the new ai system
		BehaviorTree* BT_NPC = nullptr;

		// The context that should be sent to the behaviortree
		BT_NPC_Context Context = {};

		// The playercontroller of the bot
		AFortAthenaAIBotController* PC;

		// The Pawn of the bot
		AFortPlayerPawnAthena* Pawn;

		// The PlayerState of the bot
		AFortPlayerStateAthena* PlayerState;

		// Are we ticking the bot?
		bool bTickEnabled = true;

		// So we can track the current tick that the bot is doing
		uint64_t tick_counter = 0;

	public:
		NpcBot(AFortAthenaAIBotController* PC, AFortPlayerPawnAthena* Pawn, AFortPlayerStateAthena* PlayerState)
		{
			this->PC = PC;
			this->Pawn = Pawn;
			this->PlayerState = PlayerState;

			Context.Controller = PC;
			Context.Pawn = Pawn;
			Context.PlayerState = PlayerState;
			Context.bot = this;

			NpcBots.push_back(this);
		}
	};

	BehaviorTree* ConstructBehaviorTree() {
		auto* Tree = new BehaviorTree();

		auto* RootSelector = new BTComposite_Selector();
		RootSelector->NodeName = "Alive";

		{
			auto* Selector = new BTComposite_Selector();
			Selector->NodeName = "On Ground";

			{
				auto* Task = new BTTask_Wait(0.5f);
				auto* Decorator = new BTDecorator_CheckEnum();
				Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_Global_GamePhaseStep");
				Decorator->IntValue = (int)EAthenaGamePhaseStep::EndGame;
				Decorator->Operator = EBlackboardCompareOp::GreaterThanOrEqual;
				Task->AddDecorator(Decorator);
				Selector->AddChild(Task);
			}

			{
				auto* Task = new BTTask_SteerMovement();
				auto* Service = new BTService_HandleFocusing_ScanAroundOnly();
				Task->AddService(Service);
				auto* Decorator = new BTDecorator_CheckEnum();
				Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_CharacterLaunched_ExecutionStatus");
				Decorator->IntValue = (int)EExecutionStatus::ExecutionPending;
				Decorator->Operator = EBlackboardCompareOp::GreaterThanOrEqual;
				Task->AddDecorator(Decorator);
				Selector->AddChild(Task);
			}

			{
				// Look into: FortAthenaBTTask_ShootTrap_0
				auto* Task = new BTTask_ShootTrap();
				auto* Decorator = new BTDecorator_CheckEnum();
				Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_TrapOnPath_ExecutionStatus");
				Decorator->IntValue = (int)EExecutionStatus::ExecutionPending;
				Decorator->Operator = EBlackboardCompareOp::GreaterThanOrEqual;
				Task->AddDecorator(Decorator);
				Selector->AddChild(Task);
			}

			{
				auto* Task = new BTTask_BotMoveTo();
				Task->SelectedKeyName = ConvFName(L"AIEvaluator_AvoidThreat_Destination");
				auto* Decorator = new BTDecorator_CheckEnum();
				Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_AvoidThreat_ExecutionStatus");
				Decorator->IntValue = (int)EExecutionStatus::ExecutionAllowed;
				Decorator->Operator = EBlackboardCompareOp::GreaterThanOrEqual;
				Task->AddDecorator(Decorator);
				Selector->AddChild(Task);
			}

			{
				auto* Service = new BTEvaluator_Escape_EvasiveManeuvers();
				Selector->AddService(Service);
			}

			{
				auto* Service = new BTEvaluator_CharacterLaunched();
				Selector->AddService(Service);
			}

			Tree->AllNodes.push_back(Selector);
		}

		{
			auto* Task = new BTTask_Dive();
			auto* Decorator = new BTDecorator_CheckEnum();
			Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_Dive_ExecutionStatus");
			Decorator->IntValue = 4;
			Task->AddDecorator(Decorator);
			RootSelector->AddChild(Task);
		}

		{
			auto* Task = new BTTask_Glide();
			auto* Decorator = new BTDecorator_CheckEnum();
			Decorator->SelectedKeyName = ConvFName(L"AIEvaluator_Glide_ExecutionStatus");
			Decorator->IntValue = 4;
			Task->AddDecorator(Decorator);
			RootSelector->AddChild(Task);
		}

		{
			auto* Task = new BTTask_RunSelector();
			Task->SelectorToRun = Tree->FindSelectorByName("On Ground");
			if (Task->SelectorToRun) {
				RootSelector->AddChild(Task);
			}
		}

		{
			auto* Service = new BTEvaluator_FreeFall();
			RootSelector->AddService(Service);
		}

		Tree->RootNode = RootSelector;
		Tree->AllNodes.push_back(RootSelector);

		return Tree;
	}

	void TickBots() {
		for (auto bot : NpcBots)
		{
			if (!bot->bTickEnabled) continue;

			if (bot->BT_NPC) {
				bot->BT_NPC->Tick(bot->Context);
				bot->tick_counter++;
			}
		}
	}
}