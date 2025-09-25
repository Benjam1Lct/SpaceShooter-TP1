#pragma once
	
	#include "CoreMinimal.h"
	#include "GameFramework/Pawn.h"
	#include "GameFramework/FloatingPawnMovement.h"
	
	#include "VaisseauMover.generated.h"
	
	UCLASS()
	class TP1_PROG2_API AVaisseauMover : public APawn
	{
		GENERATED_BODY()
	
	public:
		// Constructor
		AVaisseauMover();
	
		// Appliquer le mouvement avant/arrière
		void MoveForward(float Value);
	
		// Appliquer le mouvement gauche/droite
		void MoveRight(float Value);
	
		UFUNCTION()
		void Shoot();
	
		// AJOUTEZ CETTE FONCTION - Pour recevoir les dégâts
		virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
		// AJOUTEZ CETTE FONCTION - Gestion de collision
		UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	protected:
		virtual void BeginPlay() override;
	
	public:
		virtual void Tick(float DeltaTime) override;
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

		// Fonction pour déclencher l'effet de dégâts
		UFUNCTION(BlueprintCallable)
		void TriggerDamageFlash();
	
	private:
		float Speed;
		FVector MovementInput;
	
		UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* MeshComponent;
	
		UPROPERTY(VisibleAnywhere)
		UFloatingPawnMovement* MovementComponent;
	
		// Système de points de vie
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
		float Health = 300.0f;
	
		// Remplacez l'include de UBoxComponent par USphereComponent :
		UPROPERTY(VisibleAnywhere, Category = "Collision")
		class USphereComponent* CollisionComponent;

		// Variables pour l'effet de dégâts
		UPROPERTY(VisibleAnywhere, Category = "Effects")
		bool bIsFlashing = false;
    
		UPROPERTY(VisibleAnywhere, Category = "Effects")
		float FlashTimer = 0.0f;
    
		UPROPERTY(EditAnywhere, Category = "Effects")
		float FlashDuration = 0.5f; // Durée totale du clignotement
    
		UPROPERTY(EditAnywhere, Category = "Effects")
		float FlashRate = 0.1f; // Vitesse de clignotement
    
		// Matériau d'origine pour la restauration
		UPROPERTY()
		UMaterialInterface* OriginalMaterial;
    
		// Matériau rouge pour les dégâts
		UPROPERTY()
		UMaterialInstanceDynamic* DamageMaterial;

		// Système de particules Niagara pour le tir
		UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraSystem* FireEffect;

		// Timer pour l'effet de tir
		FTimerHandle ShootFlashTimer;

		// Référence au PlayerController pour obtenir la position de la souris
		UPROPERTY()
		APlayerController* PlayerController;

		// Composant pour afficher la cible
		UPROPERTY(VisibleAnywhere, Category = "UI")
		class UStaticMeshComponent* TargetCrosshair;

		// Offset pour la cible (éviter qu'elle soit dans le sol)
		UPROPERTY(EditAnywhere, Category = "UI")
		float CrosshairHeightOffset = 5.0f;

		
	};