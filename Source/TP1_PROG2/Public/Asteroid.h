#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

UCLASS()
class TP1_PROG2_API AAsteroid : public AActor
{
	GENERATED_BODY()

public:
	// Constructeur
	AAsteroid();

protected:
	// Fonction appelée au début du jeu
	virtual void BeginPlay() override;

public:
	// Fonction appelée chaque frame
	virtual void Tick(float DeltaTime) override;

	// Signature correcte de TakeDamage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Mesh de l'astéroïde
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class UBoxComponent* CollisionComponent;

	// Vie de l'astéroïde
	float Health;

	// Taille aléatoire
	float Size;

	// Type d'astéroïde : 0 pour mouvement vers le centre, 1 pour suivre le joueur
	int32 Type;

	// Vitesse de l'astéroïde
	float Speed = 100.0f;

	// Référence au Pawn du joueur (pour le suivi)
	APawn* PlayerPawn;

	// Direction de mouvement pour les astéroïdes de type 0
	FVector MovementDirection;

	// Vérifier si l'astéroïde sort des limites
	void CheckBounds();

	bool bIsDestructible = false;

	// Ajoutez cette variable pour stocker les PV initiaux
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float InitialHealth;

	// Fonction pour gérer la collision
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Fonction pour déclencher l'effet de clignotement
	UFUNCTION(BlueprintCallable)
	void TriggerDamageFlash();
	
private:
    // Système de particules d'explosion
    UPROPERTY(EditAnywhere, Category = "Effects")
    class UNiagaraSystem* ExplodeEffect;

    // Variables pour l'animation de destruction
    UPROPERTY()
    bool bIsBeingDestroyed = false;

    UPROPERTY()
    float DestroyAnimationTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Effects")
    float DestroyDuration = 0.5f; // Durée de l'animation

    UPROPERTY()
    FVector InitialScale;

    // AJOUT - Variables pour l'effet de clignotement
    UPROPERTY()
    bool bIsFlashing = false;
    
    UPROPERTY()
    float FlashTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    float FlashDuration = 0.2f; // Durée du clignotement
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    float FlashRate = 0.2f; // Vitesse de clignotement
    
    // Matériaux pour l'effet
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
    
    UPROPERTY()
    UMaterialInterface* BaseMaterial;
    
    UPROPERTY()
    UMaterialInstanceDynamic* DamageMaterial;
};
