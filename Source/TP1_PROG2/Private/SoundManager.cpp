#include "SoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
// Initialisation de l'instance statique
ASoundManager* ASoundManager::Instance = nullptr;

ASoundManager::ASoundManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Créer le composant audio pour la musique de fond
    BackgroundMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BackgroundMusic"));
    RootComponent = BackgroundMusicComponent;

    // Charger et vérifier chaque SoundWave
    SoundtrackSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Assets/soundtrack"));
    if (!SoundtrackSound)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: soundtrack non trouvé !"));
    }

    AsteroidDestroySound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Assets/asteroidandlife"));
    if (!AsteroidDestroySound)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: asteroidandlife non trouvé !"));
    }

    VaisseauHitSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Assets/vaisseauhit"));
    if (!VaisseauHitSound)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: vaisseauhit non trouvé !"));
    }

    EarnPointSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Assets/earnpoint"));
    if (!EarnPointSound)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: earnpoint non trouvé !"));
    }

    FireSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Assets/fireshoot"));
    if (!FireSound)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR: fireshoot non trouvé !"));
    }

    // Configuration de la musique de fond
    if (SoundtrackSound)
    {
        BackgroundMusicComponent->SetSound(SoundtrackSound);
        BackgroundMusicComponent->bAutoActivate = false;
        BackgroundMusicComponent->SetVolumeMultiplier(0.3f);
        BackgroundMusicComponent->bIsUISound = false;
        BackgroundMusicComponent->bShouldRemainActiveIfDropped = true;

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Soundtrack configuré avec succès !"));
        }
    }
}

void ASoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Définir cette instance comme l'instance globale
    Instance = this;

    // Démarrer la musique de fond en boucle
    if (BackgroundMusicComponent && SoundtrackSound)
    {
        BackgroundMusicComponent->Play();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Musique de fond démarrée !"));
        }
    }

    
}

void ASoundManager::PlayAsteroidDestroy()
{
    if (AsteroidDestroySound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), AsteroidDestroySound, 0.7f);
    }
}

void ASoundManager::PlayVaisseauHit()
{
    if (VaisseauHitSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), VaisseauHitSound, 0.8f);
    }
}

void ASoundManager::PlayEarnPoint()
{
    if (EarnPointSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), EarnPointSound, 0.5f);
    }
}

void ASoundManager::PlayFire()
{
    if (FireSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), FireSound, 0.6f);
    }
}