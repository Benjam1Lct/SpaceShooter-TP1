#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputHandler.generated.h"

UCLASS()
class TP1_PROG2_API AInputHandler : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AInputHandler();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Recevoir les entrées et les envoyer au VaisseauMover
	void MoveForward(float Value);
	void MoveRight(float Value);

	// Référence au VaisseauMover (sera assigné depuis le GameMode ou le niveau)
	class AVaisseauMover* VaisseauMover;  // Référence vers la classe qui déplace le vaisseau
};
