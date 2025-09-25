#pragma once
 
 #include "CoreMinimal.h"
 #include "Blueprint/UserWidget.h"
 #include "Components/TextBlock.h"
 #include "Components/Button.h"
 #include "GameOverWidget.generated.h"
 
 UCLASS()
 class TP1_PROG2_API UGameOverWidget : public UUserWidget
 {
     GENERATED_BODY()
 
 public:
     // Fonction pour afficher le menu Game Over
     UFUNCTION(BlueprintCallable, Category = "GameOver")
     void ShowGameOverMenu(int32 FinalScore, float GameTime);
 
 protected:
     virtual void NativeConstruct() override;
 
     // Éléments du menu
     UPROPERTY(meta = (BindWidget))
     class UTextBlock* GameOverText;
 
     UPROPERTY(meta = (BindWidget))
     class UTextBlock* FinalScoreText;
 
     UPROPERTY(meta = (BindWidget))
     class UTextBlock* GameTimeText;
 
     UPROPERTY(meta = (BindWidget))
     class UButton* RestartButton;
 
     UPROPERTY(meta = (BindWidget))
     class UButton* MainMenuButton;
 
     // Fonctions des boutons
     UFUNCTION()
     void OnRestartClicked();
 
     UFUNCTION()
     void OnMainMenuClicked();
 };