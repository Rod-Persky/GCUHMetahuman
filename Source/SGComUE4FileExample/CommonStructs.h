// Commonly used structures

#pragma once

#include "SG.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CommonStructs.generated.h"

// Contains SG animation data
USTRUCT(BlueprintType)
struct FAvatarInfo {
    GENERATED_BODY()

    SG_AnimationNode* AnimationNodes = nullptr;
    sg_size NumAnimationNodes = 0;
};
