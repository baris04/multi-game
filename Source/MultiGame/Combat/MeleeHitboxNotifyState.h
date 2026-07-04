#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeHitboxNotifyState.generated.h"

/**
 * Place on an attack montage to define the active hit frames. Opens the combat
 * component's damage window on Begin and closes it on End.
 */
UCLASS(meta = (DisplayName = "Melee Hitbox Window"))
class MULTIGAME_API UMeleeHitboxNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
