#include "MonGameMode.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "VaisseauMover.h"
#include "Asteroid.h"
#include "GameHUD.h"
#include "GameOverWidget.h"
#include "Kismet/GameplayStatics.h"
#include "SoundManager.h"

AMonGameMode::AMonGameMode()
{
    // Configuration existante...
    VaisseauBlueprintClass = AVaisseauMover::StaticClass();
    DefaultPawnClass = AVaisseauMover::StaticClass();

    // Activer le Tick pour le GameMode
    PrimaryActorTick.bCanEverTick = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("GameMode initialisé - Blueprint à assigner manuellement"));
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> GameHUDWidgetClass(TEXT("/Game/BP/WBP_GameHUD"));
    if (GameHUDWidgetClass.Succeeded())
    {
        GameHUDClass = GameHUDWidgetClass.Class;
    }

    // AJOUTEZ CETTE PARTIE pour le Game Over Widget
    static ConstructorHelpers::FClassFinder<UUserWidget> GameOverWidgetClass_Finder(TEXT("/Game/BP/WBP_GameOver"));
    if (GameOverWidgetClass_Finder.Succeeded())
    {
        GameOverWidgetClass = GameOverWidgetClass_Finder.Class;
    }
}

void AMonGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Enregistrer le temps de début de partie
    GameStartTime = GetWorld()->GetTimeSeconds();

    // Créer le widget HUD
    if (GameHUDClass)
    {
        GameHUDWidget = CreateWidget<UGameHUD>(GetWorld(), GameHUDClass);
        if (GameHUDWidget)
        {
            GameHUDWidget->AddToViewport();
            GameHUDWidget->UpdateScore(CurrentScore);
            GameHUDWidget->UpdateHealth(600.0f);
        }
    }

    // FORCER la création du SoundManager
    if (GetWorld())
    {
        SoundManager = GetWorld()->SpawnActor<ASoundManager>();
        if (SoundManager)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("SoundManager créé et ajouté à l'Outliner !"));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: Impossible de créer SoundManager !"));
            }
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("GameMode initialisé"));
    }
}

void AMonGameMode::TriggerGameOver()
{
    if (bGameOver) return; // Éviter les appels multiples

    bGameOver = true;

    // Calculer le temps de jeu
    float GameTime = GetWorld()->GetTimeSeconds() - GameStartTime;

    // Arrêter tous les astéroïdes
    StopAllAsteroids();

    // ARRÊTER LE SPAWN - Effacer tous les timers du GameMode
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

    // Créer et afficher le widget Game Over
    if (GameOverWidgetClass)
    {
        GameOverWidget = CreateWidget<UGameOverWidget>(GetWorld(), GameOverWidgetClass);
        if (GameOverWidget)
        {
            GameOverWidget->AddToViewport();
            GameOverWidget->ShowGameOverMenu(CurrentScore, GameTime);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
            FString::Printf(TEXT("=== GAME OVER === Score: %d, Temps: %.1fs - SPAWN ARRÊTÉ"), CurrentScore, GameTime));
    }
}

void AMonGameMode::StopAllAsteroids()
{
    // Récupérer tous les astéroïdes et les arrêter
    TArray<AActor*> AllAsteroids;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAsteroid::StaticClass(), AllAsteroids);

    for (AActor* Actor : AllAsteroids)
    {
        AAsteroid* Asteroid = Cast<AAsteroid>(Actor);
        if (Asteroid)
        {
            // Arrêter le Tick de l'astéroïde
            Asteroid->SetActorTickEnabled(false);
        }
    }
}

void AMonGameMode::AddScore(int32 Points)
{
    CurrentScore += Points;

    // Jouer le son d'earn point
    if (ASoundManager::Instance)
    {
        ASoundManager::Instance->PlayEarnPoint();
    }
    
    // Mettre à jour l'affichage du score
    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateScore(CurrentScore);
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("=== SCORE TOTAL: %d ==="), CurrentScore));
    }
}

void AMonGameMode::UpdatePlayerHealthDisplay(float CurrentHealth)
{
    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateHealth(CurrentHealth);
    }
}

void AMonGameMode::StartPlay()
{
    // Code existant...
    if (VaisseauBlueprintClass)
    {
        DefaultPawnClass = VaisseauBlueprintClass;

        if (GEngine)
        {
            FString ClassName = VaisseauBlueprintClass->GetName();
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
                FString::Printf(TEXT("Utilisation finale de: %s"), *ClassName));
        }
    }

    Super::StartPlay();

    // Configuration de la caméra existante...
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()));
        if (CameraActor)
        {
            PlayerController->SetViewTargetWithBlend(CameraActor, 0.5f);
        }
    }
}

void AMonGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Seulement vérifier si le jeu est terminé
    if (bGameOver)
    {
        return;
    }

    // Gérer le spawn des astéroïdes
    TimeSinceLastSpawn += DeltaTime;

    if (TimeSinceLastSpawn >= AsteroidSpawnRate && CurrentAsteroidCount < MaxAsteroids)
    {
        SpawnAsteroid();
        TimeSinceLastSpawn = 0.0f;
    }

    // Compter les astéroïdes actuels
    TArray<AActor*> AsteroidActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAsteroid::StaticClass(), AsteroidActors);
    CurrentAsteroidCount = AsteroidActors.Num();
}

void AMonGameMode::SpawnAsteroid()
{
    FVector SpawnLocation = GetRandomSpawnLocation();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (GetWorld())
    {
        AAsteroid* NewAsteroid = GetWorld()->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), SpawnLocation, SpawnRotation);
        
        if (NewAsteroid && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange,
                FString::Printf(TEXT("Astéroïde spawné à: X=%.0f Y=%.0f"), SpawnLocation.X, SpawnLocation.Y));
        }
    }
}

FVector AMonGameMode::GetRandomSpawnLocation()
{
    // Récupérer la caméra pour définir la zone de jeu
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(PlayerController->GetViewTarget());
        if (CameraActor)
        {
            FVector CameraLocation = CameraActor->GetActorLocation();

            // Utiliser les mêmes calculs que pour le vaisseau
            float HeightFactor = FMath::Clamp(CameraLocation.Z / 1000.0f, 0.5f, 2.0f);

            // Définir la zone de jeu
            float GameAreaX = 420.0f * HeightFactor;
            float GameAreaY = 820.0f * HeightFactor;

            // Choisir un côté aléatoire (0=haut, 1=bas, 2=gauche, 3=droite)
            int32 Side = FMath::RandRange(0, 3);
            FVector SpawnLocation = CameraLocation;

            switch (Side)
            {
                case 0: // Haut
                    SpawnLocation.X = CameraLocation.X + FMath::RandRange(-GameAreaX - SpawnDistance, GameAreaX + SpawnDistance);
                    SpawnLocation.Y = CameraLocation.Y + GameAreaY + SpawnDistance;
                    break;
                case 1: // Bas
                    SpawnLocation.X = CameraLocation.X + FMath::RandRange(-GameAreaX - SpawnDistance, GameAreaX + SpawnDistance);
                    SpawnLocation.Y = CameraLocation.Y - GameAreaY - SpawnDistance;
                    break;
                case 2: // Gauche
                    SpawnLocation.X = CameraLocation.X - GameAreaX - SpawnDistance;
                    SpawnLocation.Y = CameraLocation.Y + FMath::RandRange(-GameAreaY - SpawnDistance, GameAreaY + SpawnDistance);
                    break;
                case 3: // Droite
                    SpawnLocation.X = CameraLocation.X + GameAreaX + SpawnDistance;
                    SpawnLocation.Y = CameraLocation.Y + FMath::RandRange(-GameAreaY - SpawnDistance, GameAreaY + SpawnDistance);
                    break;
            }

            // Utiliser un Z fixe de 92
            SpawnLocation.Z = 92.0f;

            return SpawnLocation;
        }
    }

    // Fallback si pas de caméra
    return FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), 92.0f);
}
