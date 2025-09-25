#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Lier les événements des boutons
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
    }

    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
    }
}

void UGameOverWidget::ShowGameOverMenu(int32 FinalScore, float GameTime)
{
    // Afficher le score final
    if (FinalScoreText)
    {
        FinalScoreText->SetText(FText::Format(FText::FromString(TEXT("SCORE FINAL: {0}")), FinalScore));
    }

    // Convertir le temps en minutes:secondes
    if (GameTimeText)
    {
        int32 Minutes = (int32)(GameTime / 60.0f);
        int32 Seconds = (int32)(GameTime) % 60;
        FString TimeString = FString::Printf(TEXT("TEMPS DE JEU: %02d:%02d"), Minutes, Seconds);
        GameTimeText->SetText(FText::FromString(TimeString));
    }

    // Activer la souris pour les boutons
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }
}

void UGameOverWidget::OnRestartClicked()
{
    // Relancer le niveau actuel
    UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()), false);
}

void UGameOverWidget::OnMainMenuClicked()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("Retour au menu principal..."));
    }

    // Remettre la souris en mode normal avant de changer de niveau
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
    }

    // Charger le niveau MainMenu
    UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu"), false);
}