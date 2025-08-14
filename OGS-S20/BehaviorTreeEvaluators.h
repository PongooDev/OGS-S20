#pragma once
#include "framework.h"
#include "BehaviorTreeSystem.h"

class BTEvaluator_CharacterLaunched : public BTService {
public:
    BTEvaluator_CharacterLaunched() {
        NodeName = "Evaluating...Character Launched";

        Interval = 1.f;
    }

    void TickService(BTContext Context) override {
        if (!Context.Pawn || !Context.Controller) return;

        UPawnMovementComponent* MovementComp = Context.Pawn->GetMovementComponent();
        if (MovementComp) {
            if (MovementComp->IsFalling()) {
                Context.Controller->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_CharacterLaunched_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionAllowed);
            }
            else {
                Context.Controller->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_CharacterLaunched_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionDenied);
            }
        }
    }
};

class BTEvaluator_FreeFall : public BTService {
public:
	BTEvaluator_FreeFall() {
		NodeName = "Evaluating...Free Fall";
	}

	virtual void TickService(BTContext Context) override {
		if (!Context.Pawn || !Context.Controller) return;

		if (Context.Pawn->IsSkydiving())
		{
			Context.Controller->Blackboard->SetValueAsEnum(ConvFName(L"AIEvaluator_Dive_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionAllowed);
		}
		else
		{
			if (Context.Pawn->IsParachuteOpen()) {
				Context.Controller->Blackboard->SetValueAsEnum(ConvFName(L"AIEvaluator_Glide_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionAllowed);
			}
			else {
				Context.Controller->Blackboard->SetValueAsEnum(ConvFName(L"AIEvaluator_Glide_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionDenied);
			}
			Context.Controller->Blackboard->SetValueAsEnum(ConvFName(L"AIEvaluator_Dive_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionDenied);
		}
	}
};