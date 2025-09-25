#include "Asteroid.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "MonGameMode.h"
#include "Projectile.h"
#include "VaisseauMover.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"  // Ajoutez cette ligne
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "SoundManager.h"


AAsteroid::AAsteroid()
{
    PrimaryActorTick.bCanEverTick = true;

    // Créer le composant de collision en premier
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    if (CollisionComponent)
    {
        CollisionComponent->SetBoxExtent(FVector(200.0f, 200.0f, 500.0f));
        RootComponent = CollisionComponent;

        // Configuration collision pour le composant dédié
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
        CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
        CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        CollisionComponent->SetNotifyRigidBodyCollision(true);

        // Active la simulation physique
        CollisionComponent->SetSimulatePhysics(true);

        // Bloquer l'axe Z
        CollisionComponent->SetConstraintMode(EDOFMode::SixDOF);
        CollisionComponent->GetBodyInstance()->SetDOFLock(EDOFMode::SixDOF);
        CollisionComponent->GetBodyInstance()->bLockZTranslation = true;
        CollisionComponent->GetBodyInstance()->bLockXRotation = true;
        CollisionComponent->GetBodyInstance()->bLockYRotation = true;

        CollisionComponent->OnComponentHit.AddDynamic(this, &AAsteroid::OnHit);
    }

    // Créer le mesh et l'attacher au composant de collision
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(CollisionComponent);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // Désactiver les ombres
        MeshComponent->SetCastShadow(false);
        MeshComponent->bCastDynamicShadow = false;
        MeshComponent->bCastStaticShadow = false;
    }

    // Initialisation de la vie de l'astéroïde (sans LoadObject)
    int32 HealthValues[] = {100, 200, 300};
    Health = HealthValues[FMath::RandRange(0, 2)];
    InitialHealth = Health;

    // Taille aléatoire
    Size = FMath::RandRange(0.2f, .5f);
    SetActorScale3D(FVector(Size, Size, Size));

    // Vitesse
    Speed = FMath::RandRange(100.0f, 300.0f);

    // Initialiser les variables
    bIsDestructible = false;
    bIsBeingDestroyed = false;
    DestroyAnimationTime = 0.0f;
    DestroyDuration = 0.5f;
    bIsFlashing = false;
    FlashTimer = 0.0f;
    FlashDuration = 0.2f;
    FlashRate = 0.2f;
}

void AAsteroid::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Vérifier si l'objet qui nous touche est un projectile
    AProjectile* HitProjectile = Cast<AProjectile>(OtherActor);
    if (HitProjectile)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue,
                FString::Printf(TEXT("Astéroïde touché par projectile ! Vie: %.0f"), Health));
        }

        // Prendre des dégâts
        FDamageEvent DamageEvent;
        TakeDamage(100.0f, DamageEvent, nullptr, HitProjectile);

        // Détruire le projectile
        HitProjectile->Destroy();
    }

    // SUPPRIMEZ TOUTE LA PARTIE VAISSEAU - laissez le vaisseau gérer sa propre collision
}

void AAsteroid::BeginPlay()
{
    Super::BeginPlay();

    // DÉPLACER TOUS LES LOADOBJECT ICI
    if (MeshComponent)
    {
        // Charger le mesh de l'astéroïde
        UStaticMesh* AsteroidMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fab/Asteroid/Asteroid4_lowFinal.Asteroid4_lowFinal"));
        if (AsteroidMesh)
        {
            MeshComponent->SetStaticMesh(AsteroidMesh);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Mesh astéroïde chargé !"));
            }
        }
        else
        {
            // Fallback vers le cube
            UStaticMesh* FallbackMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (FallbackMesh)
            {
                MeshComponent->SetStaticMesh(FallbackMesh);
            }
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERREUR : Mesh astéroïde non trouvé, utilisation du cube"));
            }
        }

        // Sauvegarder le matériau original
        OriginalMaterial = MeshComponent->GetMaterial(0);
        
        // Créer le matériau de dégâts
        BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        if (BaseMaterial)
        {
            DamageMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DamageMaterial)
            {
                DamageMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }

    // Charger l'effet d'explosion
    ExplodeEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Assets/Explode.Explode"));
    if (ExplodeEffect)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Effet Explode chargé !"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ERREUR : Effet Explode non trouvé"));
        }
    }

    // Récupérer la position du joueur pour le type 1 d'astéroïde
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerPawn = PlayerController->GetPawn();
    }

    // Reste du code BeginPlay existant pour le type d'astéroïde...
    AMonGameMode* GameMode = Cast<AMonGameMode>(GetWorld()->GetAuthGameMode());
    
    TArray<AActor*> AllAsteroids;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAsteroid::StaticClass(), AllAsteroids);
    
    int32 Type1Count = 0;
    for (AActor* Actor : AllAsteroids)
    {
        AAsteroid* ExistingAsteroid = Cast<AAsteroid>(Actor);
        if (ExistingAsteroid && ExistingAsteroid != this && ExistingAsteroid->Type == 1)
        {
            Type1Count++;
        }
    }

    float RandomValue = FMath::RandRange(0.0f, 1.0f);

    if (Type1Count >= 3)
    {
        Type = 0;
    }
    else if (GameMode && RandomValue <= GameMode->Type0Percentage)
    {
        Type = 0;
    }
    else
    {
        Type = 1;
    }

    if (Type == 0)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(GetWorld()->GetFirstPlayerController()->GetViewTarget());
        if (CameraActor)
        {
            FVector CameraLocation = CameraActor->GetActorLocation();
            CameraLocation.Z = 92.0f;

            FVector DirectionToCenter = (CameraLocation - GetActorLocation()).GetSafeNormal();
            FVector RandomOffset = FVector(FMath::RandRange(-0.3f, 0.3f), FMath::RandRange(-0.3f, 0.3f), 0.0f);
            DirectionToCenter = (DirectionToCenter + RandomOffset).GetSafeNormal();

            MovementDirection = DirectionToCenter;
        }
    }
}

void AAsteroid::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // AJOUT - Gestion du clignotement
    if (bIsFlashing && !bIsBeingDestroyed)
    {
        FlashTimer += DeltaTime;
        
        // Calculer l'état de clignotement
        float FlashPhase = FMath::Fmod(FlashTimer, FlashRate * 2.0f);
        bool bShouldBeBlack = FlashPhase < FlashRate;
        
        // Appliquer le matériau
        if (bShouldBeBlack && DamageMaterial)
        {
            MeshComponent->SetMaterial(0, DamageMaterial);
        }
        else if (OriginalMaterial)
        {
            MeshComponent->SetMaterial(0, OriginalMaterial);
        }
        
        // Arrêter après la durée
        if (FlashTimer >= FlashDuration)
        {
            bIsFlashing = false;
            FlashTimer = 0.0f;
            
            if (OriginalMaterial)
            {
                MeshComponent->SetMaterial(0, OriginalMaterial);
            }
        }
    }

    // NOUVEAU - Gestion de l'animation de destruction
    if (bIsBeingDestroyed)
    {
        DestroyAnimationTime += DeltaTime;
        
        // Calculer le pourcentage de progression (0 à 1)
        float Progress = DestroyAnimationTime / DestroyDuration;
        
        if (Progress >= 1.0f)
        {
            // Animation terminée, détruire réellement
            Super::Destroy(); // Appeler la fonction Destroy() de la classe parent
            return;
        }
        
        // Interpolation de l'échelle (de InitialScale vers 0)
        float ScaleFactor = 1.0f - Progress;
        FVector NewScale = InitialScale * ScaleFactor;
        SetActorScale3D(NewScale);
        
        // Optionnel : rotation pendant la destruction
        FRotator CurrentRotation = GetActorRotation();
        CurrentRotation.Yaw += 360.0f * DeltaTime; // Rotation de 360° par seconde
        SetActorRotation(CurrentRotation);
        
        return; // Ne pas exécuter le mouvement normal pendant la destruction
    }

    // Mouvement normal (code existant)
    FVector NewLocation = GetActorLocation();

    if (Type == 1 && PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        PlayerLocation.Z = 92.0f;

        FVector DirectionToPlayer = (PlayerLocation - GetActorLocation()).GetSafeNormal();
        NewLocation += DirectionToPlayer * Speed * DeltaTime;
    }
    else
    {
        NewLocation += MovementDirection * Speed * DeltaTime;
    }

    SetActorLocation(NewLocation);
    CheckBounds();
}

void AAsteroid::CheckBounds()
{
    FVector Location = GetActorLocation();

    // Vérifier les limites de la zone de jeu
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(PlayerController->GetViewTarget());
        if (CameraActor)
        {
            FVector CameraLocation = CameraActor->GetActorLocation();
            float HeightFactor = FMath::Clamp(CameraLocation.Z / 1000.0f, 0.5f, 2.0f);

            float GameAreaX = 420.0f * HeightFactor;
            float GameAreaY = 820.0f * HeightFactor;

            float MinX = CameraLocation.X - GameAreaX;
            float MaxX = CameraLocation.X + GameAreaX;
            float MinY = CameraLocation.Y - GameAreaY;
            float MaxY = CameraLocation.Y + GameAreaY;

            bool bIsInGameArea = (Location.X >= MinX && Location.X <= MaxX && Location.Y >= MinY && Location.Y <= MaxY);

            if (!bIsDestructible && bIsInGameArea)
            {
                bIsDestructible = true;
            }

            if (bIsDestructible)
            {
                float DestroyMargin = 200.0f;
                float DestroyMinX = MinX - DestroyMargin;
                float DestroyMaxX = MaxX + DestroyMargin;
                float DestroyMinY = MinY - DestroyMargin;
                float DestroyMaxY = MaxY + DestroyMargin;

                if (Location.X < DestroyMinX || Location.X > DestroyMaxX || Location.Y < DestroyMinY || Location.Y > DestroyMaxY)
                {
                    Destroy();
                }
            }
        }
    }
}

float AAsteroid::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsBeingDestroyed)
    {
        return 0.0f;
    }
    
    Health -= DamageAmount;

    // AJOUT - Déclencher l'effet de clignotement
    TriggerDamageFlash();

    if (Health <= 0 && !bIsBeingDestroyed)
    {
        // Calculer le score avant de commencer la destruction
        int32 ScoreEarned = (int32)(InitialHealth * 10);

        // Récupérer le GameMode pour ajouter le score
        AMonGameMode* GameMode = Cast<AMonGameMode>(GetWorld()->GetAuthGameMode());
        if (GameMode)
        {
            GameMode->AddScore(ScoreEarned);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
                    FString::Printf(TEXT("Astéroïde en destruction ! +%d points"), ScoreEarned));
            }
        }

        // NOUVEAU - Jouer le son de destruction d'astéroïde
        if (ASoundManager::Instance)
        {
            ASoundManager::Instance->PlayAsteroidDestroy();
        }

        // NOUVEAU - Déclencher l'effet d'explosion
        if (ExplodeEffect && GetWorld())
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                ExplodeEffect,
                GetActorLocation(),
                GetActorRotation(),
                FVector(Size, Size, Size), // Taille basée sur la taille de l'astéroïde
                true // Auto-détruire
            );

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Explosion déclenchée !"));
            }
        }

        // NOUVEAU - Sauvegarder l'échelle ACTUELLE au moment de la destruction
        InitialScale = GetActorScale3D();

        // NOUVEAU - Commencer l'animation de destruction
        bIsBeingDestroyed = true;
        DestroyAnimationTime = 0.0f;

        // CORRIGÉ - Désactiver COMPLÈTEMENT les collisions ET la physique
        if (CollisionComponent)
        {
            CollisionComponent->SetSimulatePhysics(false); // AJOUT : Désactiver la physique d'abord
            CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    return DamageAmount;
}

void AAsteroid::TriggerDamageFlash()
{
    if (!bIsBeingDestroyed)
    {
        bIsFlashing = true;
        FlashTimer = 0.0f;
    }
}