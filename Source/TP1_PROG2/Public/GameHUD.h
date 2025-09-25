#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameHUD.generated.h"

UCLASS()
class TP1_PROG2_API UGameHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // Fonction pour mettre à jour le score
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateScore(int32 NewScore);

    // Fonction pour mettre à jour la vie (0-600)
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float CurrentHealth, float MaxHealth = 600.0f);

protected:
    // Texte du score
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScoreText;

    // Images des 3 cœurs (entiers + moitiés + vides)
    UPROPERTY(meta = (BindWidget))
    class UImage* Heart1_Full;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart1_Half;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart1_Empty;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart2_Full;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart2_Half;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart2_Empty;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart3_Full;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart3_Half;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart3_Empty;

private:
    // Fonction pour mettre à jour l'affichage d'un cœur
    void UpdateHeartDisplay(class UImage* FullHeart, class UImage* HalfHeart, class UImage* EmptyHeart, int32 HeartValue);
};