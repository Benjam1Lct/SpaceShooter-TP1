#include "InputHandler.h"
#include "VaisseauMover.h"

AInputHandler::AInputHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInputHandler::BeginPlay()
{
	Super::BeginPlay();
}

void AInputHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Recevoir la commande de mouvement avant/arrière
void AInputHandler::MoveForward(float Value)
{
	if (VaisseauMover)
	{
		VaisseauMover->MoveForward(Value);  // Notifier la classe VaisseauMover
	}
}

// Recevoir la commande de mouvement gauche/droite
void AInputHandler::MoveRight(float Value)
{
	if (VaisseauMover)
	{
		VaisseauMover->MoveRight(Value);  // Notifier la classe VaisseauMover
	}
}
