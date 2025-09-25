#include "VaisseauMover.h"
#include "Projectile.h"
#include "Asteroid.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "MonGameMode.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "SoundManager.h"
#include "Misc/PackageName.h"

AVaisseauMover::AVaisseauMover()
{
    PrimaryActorTick.bCanEverTick = true;

    // Créer le composant de collision sphérique - TAILLE CORRECTE
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    if (CollisionComponent)
    {
        CollisionComponent->SetSphereRadius(380.0f); // CORRIGÉ - était 380
        RootComponent = CollisionComponent;

        // Configuration collision
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetCollisionObjectType(ECC_Pawn);
        CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        CollisionComponent->SetNotifyRigidBodyCollision(true);

        // Lier l'événement de collision
        CollisionComponent->OnComponentHit.AddDynamic(this, &AVaisseauMover::OnHit);
    }

    // Création du Mesh pour le vaisseau
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(CollisionComponent);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Créer le crosshair
    TargetCrosshair = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetCrosshair"));
    if (TargetCrosshair)
    {
        TargetCrosshair->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Créer le composant de mouvement
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    if (MovementComponent)
    {
        MovementComponent->UpdatedComponent = RootComponent;
        MovementComponent->MaxSpeed = 600.0f;
    }

    // Initialisation des variables
    Speed = 600.0f;
    MovementInput = FVector::ZeroVector;
    Health = 600.0f;

    // Rotation initiale
    SetActorRotation(FRotator::ZeroRotator);
}

void AVaisseauMover::BeginPlay()
{
    Super::BeginPlay();

    // Récupérer le PlayerController
    PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());

    // Charger le mesh du vaisseau
    if (MeshComponent)
    {
        UStaticMesh* VaisseauMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/AxionClaymore/Meshes/SM_AxionClaymore.SM_AxionClaymore"));
        if (VaisseauMesh)
        {
            MeshComponent->SetStaticMesh(VaisseauMesh);
            SetActorScale3D(FVector(0.15f, 0.15f, 0.15f));
            
            FVector MeshOffset = FVector(-110.0f, 0.0f, 0.0f);
            MeshComponent->SetRelativeLocation(MeshOffset);
            
            MeshComponent->SetVisibility(true);
            MeshComponent->SetHiddenInGame(false);

            // Sauvegarder le matériau original
            OriginalMaterial = MeshComponent->GetMaterial(0);

            // Créer le matériau rouge pour les dégâts
            UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            if (BaseMaterial)
            {
                DamageMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (DamageMaterial)
                {
                    DamageMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
                }
            }
        }
    }

    // Configurer le crosshair
    if (TargetCrosshair)
    {
        UStaticMesh* CrosshairMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (CrosshairMesh)
        {
            TargetCrosshair->SetStaticMesh(CrosshairMesh);
            TargetCrosshair->SetWorldScale3D(FVector(0.3f, 0.3f, 0.3f));

            // Matériau rouge pour le crosshair
            UMaterialInterface* CrosshairBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            if (CrosshairBaseMaterial)
            {
                UMaterialInstanceDynamic* CrosshairMaterial = UMaterialInstanceDynamic::Create(CrosshairBaseMaterial, this);
                if (CrosshairMaterial)
                {
                    CrosshairMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
                    TargetCrosshair->SetMaterial(0, CrosshairMaterial);
                }
            }
        }
    }

    // Charger l'effet de tir
    FireEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Assets/Fire.Fire"));
}

void AVaisseauMover::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // FORCER LE Z À 92 - AJOUTEZ CETTE PARTIE
    FVector CurrentLocation = GetActorLocation();
    if (CurrentLocation.Z != 92.0f)
    {
        CurrentLocation.Z = 92.0f;
        SetActorLocation(CurrentLocation);
    }

    // Variables pour stocker la direction vers la souris
    FVector DirectionToMouse = FVector::ZeroVector;
    bool bMouseDirectionValid = false;
    FVector MouseWorldPosition = FVector::ZeroVector; // AJOUT


    // Orienter le vaisseau vers la souris et calculer la direction
    if (PlayerController)
    {
        FVector MouseWorldLocation, MouseWorldDirection;
        if (PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
        {
            FVector VaisseauLocation = GetActorLocation();
            float t = (VaisseauLocation.Z - MouseWorldLocation.Z) / MouseWorldDirection.Z;
            FVector MousePositionOnPlane = MouseWorldLocation + (MouseWorldDirection * t);

            // AJOUT - Stocker la position de la souris pour la cible
            MouseWorldPosition = MousePositionOnPlane;
            MouseWorldPosition.Z = 300.0f; // Élever légèrement la cible

            
            DirectionToMouse = (MousePositionOnPlane - VaisseauLocation).GetSafeNormal();
            bMouseDirectionValid = true;

            // Orienter le vaisseau vers la souris
            FRotator TargetRotation = DirectionToMouse.Rotation();
            SetActorRotation(TargetRotation);

            // AJOUT - Mettre à jour la position de la cible
            if (TargetCrosshair)
            {
                TargetCrosshair->SetWorldLocation(MouseWorldPosition);
                TargetCrosshair->SetVisibility(true);
            }
        }
    }
    else
    {
        // AJOUT - Cacher la cible si pas de PlayerController
        if (TargetCrosshair)
        {
            TargetCrosshair->SetVisibility(false);
        }
    }

    // Gestion du clignotement (code existant)
    if (bIsFlashing)
    {
        FlashTimer += DeltaTime;
        float FlashPhase = FMath::Fmod(FlashTimer, FlashRate * 2.0f);
        bool bShouldBeRed = FlashPhase < FlashRate;

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Purple,
                FString::Printf(TEXT("Flash: %.2f | Phase: %.2f | Rouge: %s"),
                FlashTimer, FlashPhase, bShouldBeRed ? TEXT("OUI") : TEXT("NON")));
        }

        if (bShouldBeRed && DamageMaterial)
        {
            MeshComponent->SetMaterial(0, DamageMaterial);
        }
        else if (OriginalMaterial)
        {
            MeshComponent->SetMaterial(0, OriginalMaterial);
        }

        if (FlashTimer >= FlashDuration)
        {
            bIsFlashing = false;
            FlashTimer = 0.0f;

            if (OriginalMaterial)
            {
                MeshComponent->SetMaterial(0, OriginalMaterial);
            }

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Clignotement terminé"));
            }
        }
    }

    // NOUVEAU - Mouvement basé sur la direction de la souris
    if (bMouseDirectionValid)
    {
        // Calculer le vecteur de mouvement
        FVector MovementVector = FVector::ZeroVector;
        
        // Forward (Z) = direction vers la souris
        MovementVector += DirectionToMouse * MovementInput.X;
        
        // Right = perpendiculaire à la direction de la souris
        FVector RightVector = FVector::CrossProduct(DirectionToMouse, FVector(0, 0, 1)).GetSafeNormal();
        MovementVector += RightVector * (-MovementInput.Y);

        // Appliquer le mouvement avec limites
        APlayerController* PlayerControllerRef = GetWorld()->GetFirstPlayerController();
        if (PlayerControllerRef)
        {
            ACameraActor* CameraActor = Cast<ACameraActor>(PlayerControllerRef->GetViewTarget());
            if (CameraActor)
            {
                FVector CameraLocation = CameraActor->GetActorLocation();
                float HeightFactor = FMath::Clamp(CameraLocation.Z / 1000.0f, 0.5f, 2.0f);

                float MinX = CameraLocation.X - (420.0f * HeightFactor);
                float MaxX = CameraLocation.X + (420.0f * HeightFactor);
                float MinY = CameraLocation.Y - (820.0f * HeightFactor);
                float MaxY = CameraLocation.Y + (820.0f * HeightFactor);

                FVector NewPosition = GetActorLocation() + MovementVector * Speed * DeltaTime;

                NewPosition.X = FMath::Clamp(NewPosition.X, MinX, MaxX);
                NewPosition.Y = FMath::Clamp(NewPosition.Y, MinY, MaxY);

                SetActorLocation(NewPosition);
            }
        }
    }
}

void AVaisseauMover::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Lier les commandes de mouvement
    PlayerInputComponent->BindAxis("MoveForward", this, &AVaisseauMover::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AVaisseauMover::MoveRight);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVaisseauMover::Shoot);
}

void AVaisseauMover::MoveForward(float Value)
{
    MovementInput.X = Value;
}

void AVaisseauMover::MoveRight(float Value)
{
    MovementInput.Y = Value;
}

void AVaisseauMover::Shoot()
{
    // Calculer la position de spawn en avant du vaisseau
    FVector ForwardVector = GetActorForwardVector();
    FVector SpawnLocation = GetActorLocation() + (ForwardVector * 90.0f);
    SpawnLocation.Z = 92.0f;
    FRotator SpawnRotation = GetActorRotation();

    // Jouer le son de tir
    if (ASoundManager::Instance)
    {
        ASoundManager::Instance->PlayFire();
    }

    // Spawner l'effet Niagara Fire
    if (FireEffect && GetWorld())
    {
        FVector EffectLocation = GetActorLocation() + (ForwardVector * 50.0f);
        EffectLocation.Z = 92.0f;

        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            FireEffect,
            EffectLocation,
            SpawnRotation,
            FVector(1.0f, 1.0f, 1.0f),
            true
        );

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Effet Fire déclenché !"));
        }
    }

    // MODIFIÉ - Flash jaune du vaisseau (utiliser le matériau de base, pas DamageMaterial)
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* ShootEffect = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (ShootEffect)
        {
            ShootEffect->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Jaune
            MeshComponent->SetMaterial(0, ShootEffect);

            // Restaurer après 0.05 secondes
            GetWorld()->GetTimerManager().SetTimer(
                ShootFlashTimer,
                [this]()
                {
                    if (OriginalMaterial && !bIsFlashing)
                    {
                        MeshComponent->SetMaterial(0, OriginalMaterial);
                    }
                },
                0.05f,
                false
            );
        }
    }

    // Spawn le projectile
    if (GetWorld())
    {
        AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(AProjectile::StaticClass(), SpawnLocation, SpawnRotation);
    }
}

void AVaisseauMover::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("VAISSEAU HIT par: %s"), OtherActor ? *OtherActor->GetName() : TEXT("NULL")));
    }

        // DEBUG - Afficher les noms de level
        FString CurrentLevelName = GetWorld()->GetMapName();
        FString ShortName = FPackageName::GetShortName(CurrentLevelName);
    
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple,
                FString::Printf(TEXT("Level complet: %s | Nom court: %s"), *CurrentLevelName, *ShortName));
        }

        // Tester plusieurs variantes du nom
        bool bIsGameLevel = (ShortName == TEXT("Game")) ||
                           (ShortName == TEXT("game")) ||
                           (CurrentLevelName.Contains(TEXT("Game"))) ||
                           (CurrentLevelName.Contains(TEXT("game")));


    // Vérifier si l'objet qui nous touche est un astéroïde
    AAsteroid* HitAsteroid = Cast<AAsteroid>(OtherActor);
    if (HitAsteroid)
    {
        // NOUVEAU - Jouer le son de collision avec le vaisseau
        if (ASoundManager::Instance)
        {
            ASoundManager::Instance->PlayVaisseauHit();
        }
        
        if (GEngine)
        {
            // Afficher les PV ACTUELS avant de prendre les dégâts
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
                FString::Printf(TEXT("*** COLLISION ! PV actuels: %.0f/600 ***"), Health));
        }

        // Prendre 100 points de dégâts
        FDamageEvent DamageEvent;
        TakeDamage(100.0f, DamageEvent, nullptr, HitAsteroid);

        // Détruire l'astéroïde
        HitAsteroid->Destroy();
    }
}

float AVaisseauMover::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    Health -= DamageAmount;

    // AJOUTEZ CETTE LIGNE - Déclencher l'effet de clignotement
    TriggerDamageFlash();

    // Mettre à jour l'affichage de la vie
    AMonGameMode* GameMode = Cast<AMonGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode)
    {
        GameMode->UpdatePlayerHealthDisplay(Health);
    }

    // Reste du code existant...
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
            FString::Printf(TEXT("=== PV APRES DEGATS: %.0f/600 ==="), Health));
    }

    if (Health <= 0)
    {
        // Arrêter le clignotement en cas de Game Over
        bIsFlashing = false;
        if (OriginalMaterial)
        {
            MeshComponent->SetMaterial(0, OriginalMaterial);
        }
        
        SetActorTickEnabled(false);

        if (GameMode)
        {
            GameMode->TriggerGameOver();
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                TEXT("*** GAME OVER - VAISSEAU DETRUIT ! ***"));
        }
    }

    return DamageAmount;
}

void AVaisseauMover::TriggerDamageFlash()
{
    bIsFlashing = true;
    FlashTimer = 0.0f;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Effet de dégâts déclenché !"));
    }
}

