// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/ActorComponent.h"

#include "MetaXRAudioAcousticSettings.generated.h"

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta = (BlueprintSpawnableComponent, DisplayName = "Meta XR Audio Acoustic Settings"),
    BlueprintType)
class METAXRAUDIO_API UMetaXRAudioAcousticSettings : public UActorComponent {
  GENERATED_BODY()

 public:
  UMetaXRAudioAcousticSettings();
  void BeginDestroy() override;
  void UpdateAcousticSettings();
  static UMetaXRAudioAcousticSettings* GetActiveAcousticSettings();

#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

  // When enabled, all XR Audio Sources with Early Reflections enabled will have audible reflections
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetEarlyReflectionsEnabled,
      BlueprintGetter = GetEarlyReflectionsEnabled,
      EditAnywhere,
      Category = "Room Acoustics")
  bool EarlyReflectionsEnabled;

  // Set Early Reflections to be enabled or disabled
  UFUNCTION(BlueprintSetter)
  void SetEarlyReflectionsEnabled(bool NewEarlyReflectionsEnabled);

  // Get if the Early Reflections are enabled or disabled
  UFUNCTION(BlueprintGetter)
  bool GetEarlyReflectionsEnabled() const;

  // When enabled, all XR Audio Sources with Reverb enabled will have audible reverb
  UPROPERTY(GlobalConfig, BlueprintSetter = SetReverbEnabled, BlueprintGetter = GetReverbEnabled, EditAnywhere, Category = "Room Acoustics")
  bool ReverbEnabled;

  // Set the room acoustics control for front wall material
  UFUNCTION(BlueprintSetter)
  void SetReverbEnabled(bool NewReverbEnabled);

  // Get the room acoustics control for front wall material
  UFUNCTION(BlueprintGetter)
  bool GetReverbEnabled() const;

  // Increases the reverb level of all sound sources in the scene that have reverb enabled
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetReverbLevelDecibels,
      BlueprintGetter = GetReverbLevelDecibels,
      EditAnywhere,
      Category = "Room Acoustics",
      meta = (ClampMin = "-60.0", ClampMax = "20.0", UIMin = "-60.0", UIMax = "20.0"))
  float ReverbLevelDecibels;

  // Set the room acoustics control for back wall material
  UFUNCTION(BlueprintSetter)
  void SetReverbLevelDecibels(float NewReverbLevelDecibels);

  // Get the room acoustics control for back wall material
  UFUNCTION(BlueprintGetter)
  float GetReverbLevelDecibels() const;

 private:
  inline static UMetaXRAudioAcousticSettings* ActiveAcousticSettings = nullptr;
};
