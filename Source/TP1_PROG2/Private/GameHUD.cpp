#include "GameHUD.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UGameHUD::UpdateScore(int32 NewScore)
{
    if (ScoreText)
    {
        ScoreText->SetText(FText::Format(FText::FromString(TEXT("SCORE: {0}")), NewScore));
    }
}

void UGameHUD::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    // Calculer le nombre de demi-cœurs (600 PV = 6 demi-cœurs = 3 cœurs entiers)
    int32 HalfHearts = FMath::Clamp((int32)(CurrentHealth / 100.0f), 0, 6);

    // Calculer l'état de chaque cœur (0=vide, 1=demi, 2=plein)
    int32 Heart1State = FMath::Clamp(HalfHearts, 0, 2);
    int32 Heart2State = FMath::Clamp(HalfHearts - 2, 0, 2);
    int32 Heart3State = FMath::Clamp(HalfHearts - 4, 0, 2);

    // Mettre à jour l'affichage de chaque cœur
    UpdateHeartDisplay(Heart1_Full, Heart1_Half, Heart1_Empty, Heart1State);
    UpdateHeartDisplay(Heart2_Full, Heart2_Half, Heart2_Empty, Heart2State);
    UpdateHeartDisplay(Heart3_Full, Heart3_Half, Heart3_Empty, Heart3State);
}

void UGameHUD::UpdateHeartDisplay(UImage* FullHeart, UImage* HalfHeart, UImage* EmptyHeart, int32 HeartValue)
{
    if (!FullHeart || !HalfHeart || !EmptyHeart) return;

    switch (HeartValue)
    {
        case 0: // Cœur vide (gris) - MODIFIÉ
            FullHeart->SetVisibility(ESlateVisibility::Hidden);
            HalfHeart->SetVisibility(ESlateVisibility::Hidden);
            EmptyHeart->SetVisibility(ESlateVisibility::Visible); // Afficher le cœur vide au lieu de le cacher
            break;

        case 1: // Demi-cœur
            FullHeart->SetVisibility(ESlateVisibility::Hidden);
            HalfHeart->SetVisibility(ESlateVisibility::Visible);
            EmptyHeart->SetVisibility(ESlateVisibility::Hidden);
            break;

        case 2: // Cœur plein
            FullHeart->SetVisibility(ESlateVisibility::Visible);
            HalfHeart->SetVisibility(ESlateVisibility::Hidden);
            EmptyHeart->SetVisibility(ESlateVisibility::Hidden);
            break;
    }
}