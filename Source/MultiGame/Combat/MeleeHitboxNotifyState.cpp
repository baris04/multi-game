#include "Combat/MeleeHitboxNotifyState.h"
#include "Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UMeleeHitboxNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UCombatComponent>())
		{
			Combat->OpenDamageWindow();
		}
	}
}

void UMeleeHitboxNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UCombatComponent>())
		{
			Combat->CloseDamageWindow();
		}
	}
}
