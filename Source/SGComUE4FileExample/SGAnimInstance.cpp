#include "SGAnimInstance.h"

#include "SGComManager.h"

// ========================================================
// Evaluate
// ========================================================
bool FSGAnimInstanceProxy::Evaluate(FPoseContext& Output) {
 
    USkeletalMeshComponent* MySkeletalMeshComponent = GetSkelMeshComponent();

    if (FSGComManager::IsEngineValid())
    {
        if (AnimationNodes.Nodes == nullptr && AnimationNodes.NumNodes == 0)
        {
            FAvatarInfo AvatarInfo;
            FSGComManager::GetAnimationNodes(AvatarInfo);

            AnimationNodes.Nodes = AvatarInfo.AnimationNodes;
            AnimationNodes.NumNodes = AvatarInfo.NumAnimationNodes;
        }
    }     
    else
    {
        AnimationNodes.Nodes = nullptr;
        AnimationNodes.NumNodes = 0;
    }

    if (AnimationNodes.NumNodes == 0) {
        return false;
    }

    static bool once = true;
    USkeleton* MySkeleton = Output.AnimInstanceProxy->GetSkeleton();
    for (size_t i = 0; i < AnimationNodes.NumNodes; ++i) {
        SG_AnimationNode AnimationNode = AnimationNodes.Nodes[i];
        float* AnimationData = AnimationNode.channel_values;

        if (AnimationNode.type == SG_JOINT) {
            FVector l = FVector(AnimationData[0], -AnimationData[1], AnimationData[2]);
            FQuat q = FQuat::MakeFromEuler(FVector(AnimationData[3], -AnimationData[4], -AnimationData[5]));
            FVector s = FVector(AnimationData[6], AnimationData[7], AnimationData[8]);

            int PoseBoneIndex = Output.Pose.GetBoneContainer().GetPoseBoneIndexForBoneName(AnimationNode.name);
            if (PoseBoneIndex < 0) {
                if (once) {
                    UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Node name %s was not found."), *FString(AnimationNode.name));
                }
                continue;
            }
            int skel_idx = Output.Pose.GetBoneContainer().GetPoseToSkeletonBoneIndexArray()[PoseBoneIndex];
            FCompactPoseBoneIndex idx = Output.Pose.GetBoneContainer().GetCompactPoseIndexFromSkeletonIndex(skel_idx);

            Output.Pose[idx].AddToTranslation(l);
            Output.Pose[idx].ConcatenateRotation(q);
            Output.Pose[idx].SetScale3D(Output.Pose[idx].GetScale3D() * s);
        }
        else if (AnimationNode.type == SG_BLENDSHAPE) {
            for (uint32_t j = 0; j < AnimationNode.num_channels; ++j) {                
                FName Name = AnimationNode.channel_names[j];
                MySkeletalMeshComponent->SetMorphTarget(Name, AnimationData[j], false);

                if (once) {
                    if (MySkeletalMeshComponent->FindMorphTarget(Name) == NULL) {
                        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Morph target name %s was not found."), *Name.ToString());
                    }
                }
            }
        }
        else if (AnimationNode.type == SG_OTHER_ANIMATION_NODE) { // This only works for AnimCurves
            for (uint32_t j = 0; j < AnimationNode.num_channels; ++j) {                
                FName CurveName = FName(FString(AnimationNode.name) + FString("_") + FString(AnimationNode.channel_names[j])); // Specific to Metahumans
		        SmartName::UID_Type NameUID = MySkeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName);
                if (NameUID != SmartName::MaxUID) {                    
                    Output.Curve.Set(NameUID, AnimationNode.channel_values[j]);
                }
                else {
                    if (once) {
                        UE_LOG(
                            LogTemp,
                            Warning,
                            TEXT("[SG_COM] : Animation curve %s_%s was not found."),
                            *FString(AnimationNode.name),
                            *FString(AnimationNode.channel_names[j]));
                    }                    
                }
            }            
        }
        else {
        }
    }
    once = false;

    return true;
}

// ========================================================
// Constructor
// ========================================================
USGAnimInstance::USGAnimInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Proxy.SGAnimInstance = this;
}
