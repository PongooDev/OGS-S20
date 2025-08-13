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