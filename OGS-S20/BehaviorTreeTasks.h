#pragma once
#include "framework.h"
#include "BehaviorTreeSystem.h"

class BTTask_Wait : public BTNode {
public:
    float WaitTime = 0.f;
    float WorldWaitTime = 0.f;
    bool bFinishedWait = false;
public:
    BTTask_Wait(float InWaitTime) {
        WaitTime = InWaitTime;
        WorldWaitTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + InWaitTime;
    }

    EBTNodeResult ChildTask(BTContext Context) override
    {
        if (WaitTime == 0.f || WorldWaitTime == 0.f) {
            return EBTNodeResult::Failed;
        }
        if (UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) >= WorldWaitTime) {
            if (bFinishedWait) {
                WorldWaitTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + WaitTime;
                bFinishedWait = false;
                return EBTNodeResult::InProgress;
            }
            bFinishedWait = true;
            if (!NodeName.empty())
            {
                Log("BTTask_Wait Finished Wait For NodeName: " + NodeName);
            }
            return EBTNodeResult::Succeeded;
        }
        return EBTNodeResult::InProgress;
    }
};

class BTTask_BotMoveTo : public BTNode {
public:
    float AcceptableRadius = 100.f;
    bool bAllowStrafe = true;
    bool bStopOnOverlapNeedsUpdate = false;
    bool bUsePathfinding = false;
    bool bProjectDestinationToNavigation = false;
    bool bAllowPartialPath = false;

    bool bShouldSetFocalPoint = true;

    FName SelectedKeyName;
    FName MovementResultKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_MovementResult");
public:
    EBTNodeResult ChildTask(BTContext Context) override {
        if (!Context.Controller) {
            return EBTNodeResult::Failed;
        }

        FVector Dest = Context.Controller->Blackboard->GetValueAsVector(SelectedKeyName);
        if (Dest.IsZero()) return EBTNodeResult::Failed;

        if (bShouldSetFocalPoint)
        {
            Context.Controller->K2_SetFocalPoint(Dest);
        }
        EPathFollowingRequestResult RequestResult = Context.Controller->MoveToLocation(Dest, AcceptableRadius, bStopOnOverlapNeedsUpdate, bUsePathfinding, bProjectDestinationToNavigation, bAllowStrafe, nullptr, bAllowPartialPath);
        Context.Controller->Blackboard->SetValueAsEnum(MovementResultKey, (uint8)RequestResult);
        if (RequestResult == EPathFollowingRequestResult::Failed) {
            return EBTNodeResult::Failed;
        }

        if (RequestResult == EPathFollowingRequestResult::RequestSuccessful) {
            return EBTNodeResult::InProgress;
        }

        return EBTNodeResult::Succeeded;
    }
};

class BTTask_SteerMovement : public BTNode {
public:
    float RandDirOffset = 600.f;
    float DirectionChangeInterval = 1.f;
    float NextDirectionChangeTime = 0.f;
public:
    BTTask_SteerMovement(float Offset = 600.f, float DestChangeInterval = 1.f)
    {
        RandDirOffset = Offset;
        DirectionChangeInterval = DestChangeInterval;
    }

    EBTNodeResult ChildTask(BTContext Context) override {
        if (!Context.Pawn || !Context.Controller) {
            return EBTNodeResult::Failed;
        }
        float CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());

        if (DirectionChangeInterval == 0.f || CurrentTime >= NextDirectionChangeTime)
        {
            FVector OffsetLoc = Context.Pawn->K2_GetActorLocation();
            OffsetLoc.X += UKismetMathLibrary::RandomFloatInRange((RandDirOffset * -1.f), RandDirOffset);
            OffsetLoc.Y += UKismetMathLibrary::RandomFloatInRange((RandDirOffset * -1.f), RandDirOffset);

            EPathFollowingRequestResult Result = Context.Controller->MoveToLocation(OffsetLoc, (RandDirOffset / 10), false, false, false, true, nullptr, true);
            NextDirectionChangeTime = CurrentTime + DirectionChangeInterval;

            if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
            {
                return EBTNodeResult::Succeeded;
            }
            else if (Result == EPathFollowingRequestResult::RequestSuccessful)
            {
                return EBTNodeResult::InProgress;
            }
        }

        return EBTNodeResult::Failed;
    }
};