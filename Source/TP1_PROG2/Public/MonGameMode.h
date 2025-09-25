#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MonGameMode.generated.h"

UCLASS()
class TP1_PROG2_API AMonGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMonGameMode();

    // Nouveau paramètre pour contrôler le ratio des types d'astéroïdes - DÉPLACÉ ICI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asteroids", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Type0Percentage = 0.7f; // 70% de type 0, 30% de type 1

    // Variable de score
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 CurrentScore = 0;
    
    // Fonction pour ajouter du score
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddScore(int32 Points);

    // Référence au widget HUD
    UPROPERTY(BlueprintReadWrite, Category = "HUD")
    class UGameHUD* GameHUDWidget;

    // Classe du widget HUD à assigner dans le Blueprint
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
    TSubclassOf<class UGameHUD> GameHUDClass;

    // Fonction pour mettre à jour la vie du joueur dans le HUD
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdatePlayerHealthDisplay(float CurrentHealth);

    // Variables de Game Over
    UPROPERTY(BlueprintReadWrite, Category = "GameOver")
    bool bGameOver = false;

    UPROPERTY(BlueprintReadWrite, Category = "GameOver")
    float GameStartTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "GameOver")
    class UGameOverWidget* GameOverWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameOver")
    TSubclassOf<class UGameOverWidget> GameOverWidgetClass;

    // Fonction pour déclencher le Game Over
    UFUNCTION(BlueprintCallable, Category = "GameOver")
    void TriggerGameOver();

    // Fonction pour arrêter tous les astéroïdes
    UFUNCTION(BlueprintCallable, Category = "GameOver")
    void StopAllAsteroids();

protected:
    virtual void StartPlay() override;
    virtual void BeginPlay() override;
 
    virtual void Tick(float DeltaTime) override;

    // Paramètre pour assigner le Blueprint depuis l'éditeur
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaisseau")
    TSubclassOf<APawn> VaisseauBlueprintClass;

    // Système de spawn d'astéroïdes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asteroids")
    float AsteroidSpawnRate = 1.0f; // Spawn toutes les 2 secondes
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asteroids")
    int32 MaxAsteroids = 30; // Nombre maximum d'astéroïdes en même temps
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asteroids")
    float SpawnDistance = 500.0f; // Distance autour de la zone de jeu

private:
    float TimeSinceLastSpawn = 0.0f;
    int32 CurrentAsteroidCount = 0;

    void SpawnAsteroid();
    FVector GetRandomSpawnLocation();

    // Référence au gestionnaire de sons
    UPROPERTY()
    class ASoundManager* SoundManager;
    
};