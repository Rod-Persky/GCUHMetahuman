// Applies SG animation data to the model

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "CommonStructs.h"
#include "SGAnimInstance.generated.h"

struct SGAnimationNodes {
    SG_AnimationNode* Nodes = nullptr;
    sg_size NumNodes = 0;
};

class USGAnimInstance;
struct FSGAnimInstanceProxy : public FAnimInstanceProxy
{
	FSGAnimInstanceProxy() : FAnimInstanceProxy(), SGAnimInstance(nullptr) { UE_LOG(LogTemp, Warning, TEXT("FSGAnimInstanceProxy::Constructor 1")); }
	FSGAnimInstanceProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance), SGAnimInstance(nullptr) { UE_LOG(LogTemp, Warning, TEXT("FSGAnimInstanceProxy::Constructor 2")); }
    
	virtual bool Evaluate(FPoseContext& Output) override;

	USGAnimInstance* SGAnimInstance;

    SGAnimationNodes AnimationNodes;
};


UCLASS()
class SGCOMUE4FILEEXAMPLE_API USGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    USGAnimInstance(const FObjectInitializer& ObjectInitializer);

private:
    FAnimInstanceProxy* CreateAnimInstanceProxy() override { return& Proxy; }
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

    FSGAnimInstanceProxy Proxy;
};
