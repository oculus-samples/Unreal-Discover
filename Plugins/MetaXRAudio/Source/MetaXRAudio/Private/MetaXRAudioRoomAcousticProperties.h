// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/ActorComponent.h"
#include "MetaXR_Audio.h"

#include "MetaXRAudioRoomAcousticProperties.generated.h"

// Must match ovrAudioMaterialPreset enums
UENUM(BlueprintType)
enum class EMetaXRAudioMaterialPreset : uint8 {
  AcousticTile,
  Brick,
  BrickPainted,
  Cardboard,
  Carpet,
  CarpetHeavy,
  CarpetHeavyPadded,
  CeramicTile,
  Concrete,
  ConcreteRough,
  ConcreteBlock,
  ConcreteBlockPainted,
  Curtain,
  Foliage,
  Glass,
  GlassHeavy,
  Grass,
  Gravel,
  GypsumBoard,
  Marble,
  Mud,
  PlasterOnBrick,
  PlasterOnConcreteBlock,
  Rubber,
  Soil,
  SoundProof,
  Snow,
  Steel,
  Stone,
  Vent,
  Water,
  WoodThin,
  WoodThick,
  WoodFloor,
  WoodOnConcrete
};

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta = (BlueprintSpawnableComponent, DisplayName = "Meta XR Audio Room Acoustics Properties"),
    BlueprintType)
class METAXRAUDIO_API UMetaXRAudioRoomAcousticProperties : public UActorComponent {
  GENERATED_BODY()

 public:
  UMetaXRAudioRoomAcousticProperties();
  void BeginDestroy() override;
  void UpdateRoomModel(ovrAudioContext Context);
  static UMetaXRAudioRoomAcousticProperties* GetActiveRoomAcoustics();

#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

  // Center the room model on the listener. When disabled, center the room model on the GameObject this script is attached to.
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetLockPositionToListener,
      BlueprintGetter = GetLockPositionToListener,
      EditAnywhere,
      Category = Settings)
  bool LockPositionToListener;

  // Set the room acoustics control to lock the position to the listener
  UFUNCTION(BlueprintSetter)
  void SetLockPositionToListener(bool NewLockPositionToListener);

  // Get the room acoustics control to see if the position is locked to the listener
  UFUNCTION(BlueprintGetter)
  bool GetLockPositionToListener() const;

  // Diffuses the reflections and reverberation to simulate objects inside the room. Zero represents a completely empty room.
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetClutterFactor,
      BlueprintGetter = GetClutterFactor,
      EditAnywhere,
      Category = Settings,
      meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float ClutterFactor;

  // Set the room acoustics control for clutter factor
  UFUNCTION(BlueprintSetter)
  void SetClutterFactor(float NewClutterFactor);

  // Get the room acoustics control for clutter factor
  UFUNCTION(BlueprintGetter)
  float GetClutterFactor() const;

  // Width of the room model in meters
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetWidth,
      BlueprintGetter = GetWidth,
      EditAnywhere,
      Category = "RoomDimensions",
      meta = (ClampMin = "0.0001"))
  float Width;

  // Set the room acoustics control for width
  UFUNCTION(BlueprintSetter)
  void SetWidth(float NewWidth);

  // Get the room acoustics control for width
  UFUNCTION(BlueprintGetter)
  float GetWidth() const;

  // Height of the room model in meters
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetHeight,
      BlueprintGetter = GetHeight,
      EditAnywhere,
      Category = "RoomDimensions",
      meta = (ClampMin = "0.0001"))
  float Height;

  // Set the room acoustics control for Height
  UFUNCTION(BlueprintSetter)
  void SetHeight(float NewHeight);

  // Get the room acoustics control for Height
  UFUNCTION(BlueprintGetter)
  float GetHeight() const;

  // Depth of the room model in meters
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetDepth,
      BlueprintGetter = GetDepth,
      EditAnywhere,
      Category = "RoomDimensions",
      meta = (ClampMin = "0.0001"))
  float Depth;

  // Set the room acoustics control for Depth
  UFUNCTION(BlueprintSetter)
  void SetDepth(float NewDepth);

  // Get the room acoustics control for Depth
  UFUNCTION(BlueprintGetter)
  float GetDepth() const;

  // Material of the left wall of the room model
  UPROPERTY(GlobalConfig, BlueprintSetter = SetLeftMaterial, BlueprintGetter = GetLeftMaterial, EditAnywhere, Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset LeftMaterial;

  // Set the room acoustics control for left wall material
  UFUNCTION(BlueprintSetter)
  void SetLeftMaterial(EMetaXRAudioMaterialPreset NewLeftMaterial);

  // Get the room acoustics control for left wall material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetLeftMaterial() const;

  // Material of the right wall of the room model
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetRightMaterial,
      BlueprintGetter = GetRightMaterial,
      EditAnywhere,
      Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset RightMaterial;

  // Set the room acoustics control for right wall material
  UFUNCTION(BlueprintSetter)
  void SetRightMaterial(EMetaXRAudioMaterialPreset NewRightMaterial);

  // Get the room acoustics control for right wall material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetRightMaterial() const;

  // Material of the ceiling of the room model
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetCeilingMaterial,
      BlueprintGetter = GetCeilingMaterial,
      EditAnywhere,
      Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset CeilingMaterial;

  // Set the room acoustics control for ceiling material
  UFUNCTION(BlueprintSetter)
  void SetCeilingMaterial(EMetaXRAudioMaterialPreset NewCeilingMaterial);

  // Get the room acoustics control for ceiling material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetCeilingMaterial() const;

  // Material of the floor of the room model
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetFloorMaterial,
      BlueprintGetter = GetFloorMaterial,
      EditAnywhere,
      Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset FloorMaterial;

  // Set the room acoustics control for floor material
  UFUNCTION(BlueprintSetter)
  void SetFloorMaterial(EMetaXRAudioMaterialPreset NewFloorMaterial);

  // Get the room acoustics control for floor material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetFloorMaterial() const;

  // Material of the front wall of the room model
  UPROPERTY(
      GlobalConfig,
      BlueprintSetter = SetFrontMaterial,
      BlueprintGetter = GetFrontMaterial,
      EditAnywhere,
      Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset FrontMaterial;

  // Set the room acoustics control for front wall material
  UFUNCTION(BlueprintSetter)
  void SetFrontMaterial(EMetaXRAudioMaterialPreset NewFrontMaterial);

  // Get the room acoustics control for front wall material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetFrontMaterial() const;

  // Material of the back wall of the room model
  UPROPERTY(GlobalConfig, BlueprintSetter = SetBackMaterial, BlueprintGetter = GetBackMaterial, EditAnywhere, Category = "SurfaceMaterials")
  EMetaXRAudioMaterialPreset BackMaterial;

  // Set the room acoustics control for back wall material
  UFUNCTION(BlueprintSetter)
  void SetBackMaterial(EMetaXRAudioMaterialPreset NewBackMaterial);

  // Get the room acoustics control for back wall material
  UFUNCTION(BlueprintGetter)
  EMetaXRAudioMaterialPreset GetBackMaterial() const;

  // This function allows easy editing of the global Room Acoustic Properties component
  UFUNCTION(BlueprintCallable, Category = "MetaXRAudioRoomAcousticProperties")
  void SetRoomAcousticProperties(
      const bool NewLockPositionToListener,
      const float NewClutterFactor,
      const float NewWidth,
      const float NewHeight,
      const float NewDepth,
      const EMetaXRAudioMaterialPreset NewLeftMaterial,
      const EMetaXRAudioMaterialPreset NewRightMaterial,
      const EMetaXRAudioMaterialPreset NewCeilingMaterial,
      const EMetaXRAudioMaterialPreset NewFloorMaterial,
      const EMetaXRAudioMaterialPreset NewFrontMaterial,
      const EMetaXRAudioMaterialPreset NewBackMaterial);

 private:
  void GetWallMaterialAudioBands(EMetaXRAudioMaterialPreset MaterialPreset, ovrAudioBands Bands);
  inline static UMetaXRAudioRoomAcousticProperties* ActiveAcoustics = nullptr;
};
