#include "Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Asteroid.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"  // Ajout pour la collision sphérique

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetSphereRadius(15.0f);
    RootComponent = CollisionComponent;

    // Configuration collision - IGNORER les Pawns (vaisseau)
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // AJOUTEZ CETTE LIGNE
    CollisionComponent->SetNotifyRigidBodyCollision(true);

    // Reste de la configuration physique...
    CollisionComponent->SetSimulatePhysics(true);
    CollisionComponent->SetConstraintMode(EDOFMode::SixDOF);
    CollisionComponent->GetBodyInstance()->SetDOFLock(EDOFMode::SixDOF);
    CollisionComponent->GetBodyInstance()->bLockZTranslation = true;
    CollisionComponent->GetBodyInstance()->bLockXRotation = true;
    CollisionComponent->GetBodyInstance()->bLockYRotation = true;

    CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

      // Créer le mesh et l'attacher au composant de collision
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(CollisionComponent);
    
    // Désactiver les collisions sur le mesh (seul le CollisionComponent gère ça)
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Speed = 800.0f;



    // Load the mesh for the projectile
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
        SetActorScale3D(FVector(0.3f, 0.3f, 0.3f));

        // Créer un matériau rouge
        UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* RedMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (RedMaterial)
            {
                RedMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
                MeshComponent->SetMaterial(0, RedMaterial);
            }
        }
    }
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);



    // Move the projectile forward
    FVector NewLocation = GetActorLocation() + GetActorForwardVector() * Speed * DeltaTime;
    SetActorLocation(NewLocation);

    CheckBounds();
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Log général pour tout ce qui est touché
    if (GEngine && OtherActor)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("PROJECTILE TOUCHÉ: %s"), *OtherActor->GetName()));
    }

    // Vérifier si l'objet touché est un astéroïde
    AAsteroid* HitAsteroid = Cast<AAsteroid>(OtherActor);
    if (HitAsteroid)
    {
        // Infliger 100 points de dégâts à l'astéroïde
        FDamageEvent DamageEvent;
        HitAsteroid->TakeDamage(100.0f, DamageEvent, nullptr, this);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,
                FString::Printf(TEXT("Projectile touché astéroïde Type %d ! Vie restante: %.0f"),
                HitAsteroid->Type, HitAsteroid->Health));
        }

        // Détruire le projectile
        Destroy();
    }
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}



void AProjectile::CheckBounds()
{
    FVector Location = GetActorLocation();

    // Récupérer la caméra pour définir les limites dynamiques
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(PlayerController->GetViewTarget());
        if (CameraActor)
        {
            FVector CameraLocation = CameraActor->GetActorLocation();

            // Calculer le même facteur que dans VaisseauMover
            float HeightFactor = FMath::Clamp(CameraLocation.Z / 1000.0f, 0.5f, 2.0f);

            // Définir les limites avec une marge supplémentaire pour les projectiles
            float MarginOffset = 100.0f;
            float MinX = CameraLocation.X - (420.0f * HeightFactor) - MarginOffset;
            float MaxX = CameraLocation.X + (420.0f * HeightFactor) + MarginOffset;
            float MinY = CameraLocation.Y - (820.0f * HeightFactor) - MarginOffset;
            float MaxY = CameraLocation.Y + (820.0f * HeightFactor) + MarginOffset;

            // Détruire le projectile s'il sort des limites
            if (Location.X < MinX || Location.X > MaxX || Location.Y < MinY || Location.Y > MaxY)
            {
                if (GEngine)
                {
                    //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
                    //    FString::Printf(TEXT("Projectile détruit à: X=%.0f Y=%.0f"), Location.X, Location.Y));
                }
                Destroy();
            }
        }
    }
    else
    {
        // Fallback avec des limites fixes si pas de caméra trouvée
        if (Location.X < -3000 || Location.X > 3000 || Location.Y < -3000 || Location.Y > 3000)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Projectile détruit (fallback)"));
            }
            Destroy();
        }
    }
}