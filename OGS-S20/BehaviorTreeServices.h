#pragma once
#include "framework.h"
#include "BehaviorTreeSystem.h"

class BTService_HandleFocusing_ScanAroundOnly : public BTService {
public:
    float Offset = 100.f;
public:
    BTService_HandleFocusing_ScanAroundOnly(float InInterval = 0.3f, float InOffset = 100.f) {
        NodeName = "Focus Scan Around Only";

        Interval = InInterval;
        Offset = InOffset;
    }

    void TickService(BTContext Context) override {
        if (!Context.Pawn || !Context.Controller) return;
        auto Math = UKismetMathLibrary::GetDefaultObj();

        auto BotPos = Context.Pawn->K2_GetActorLocation();
        FVector FocusLocation = Context.Pawn->K2_GetActorLocation();
        FocusLocation.X += UKismetMathLibrary::RandomFloatInRange((Offset * -1.f), Offset);
        FocusLocation.Y += UKismetMathLibrary::RandomFloatInRange((Offset * -1.f), Offset);
        FocusLocation.Z += UKismetMathLibrary::RandomFloatInRange(((Offset / 2) * -1.f), (Offset / 2));

        FRotator Rot = Math->FindLookAtRotation(BotPos, FocusLocation);

        Context.Controller->SetControlRotation(Rot);
        Context.Controller->K2_SetActorRotation(Rot, true);

        Context.Controller->K2_SetFocalPoint(FocusLocation);
    }
};