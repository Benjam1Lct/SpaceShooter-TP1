#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class TP1_PROG2_API AProjectile : public AActor
{
	GENERATED_BODY()
    
public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

public: 
	virtual void Tick(float DeltaTime) override;

	// Speed of the projectile
	UPROPERTY(EditAnywhere)
	float Speed = 1000.0f;

	// Mesh for the projectile
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	// Dans Projectile.h, ajoutez après MeshComponent :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class USphereComponent* CollisionComponent;

	// Fonction pour gérer la collision avec les astéroïdes
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Destroy the projectile when it leaves the screen
	void CheckBounds();
};
