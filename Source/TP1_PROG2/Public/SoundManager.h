#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SoundManager.generated.h"

UCLASS()
class TP1_PROG2_API ASoundManager : public AActor
{
    GENERATED_BODY()

public:
    ASoundManager();

    // Fonction pour jouer les différents sons
    UFUNCTION(BlueprintCallable)
    void PlayAsteroidDestroy();

    UFUNCTION(BlueprintCallable)
    void PlayVaisseauHit();

    UFUNCTION(BlueprintCallable)
    void PlayEarnPoint();

    UFUNCTION(BlueprintCallable)
    void PlayFire();

    // Instance statique pour accès global
    static ASoundManager* Instance;

protected:
    virtual void BeginPlay() override;

private:
        // Composant audio pour la musique de fond
        UPROPERTY(VisibleAnywhere, Category = "Audio")
        UAudioComponent* BackgroundMusicComponent;
    
        // Sons du jeu - MODIFIÉ pour USoundWave
        UPROPERTY(EditAnywhere, Category = "Sounds")
        USoundWave* SoundtrackSound;
    
        UPROPERTY(EditAnywhere, Category = "Sounds")
        USoundWave* AsteroidDestroySound;
    
        UPROPERTY(EditAnywhere, Category = "Sounds")
        USoundWave* VaisseauHitSound;
    
        UPROPERTY(EditAnywhere, Category = "Sounds")
        USoundWave* EarnPointSound;
    
        UPROPERTY(EditAnywhere, Category = "Sounds")
        USoundWave* FireSound;
};