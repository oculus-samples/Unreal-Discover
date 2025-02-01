// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAudioRoomAcousticProperties.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioPlatform.h"
#include "MetaXRAudioUtilities.h"

UMetaXRAudioRoomAcousticProperties::UMetaXRAudioRoomAcousticProperties()
    : LockPositionToListener(true),
      ClutterFactor(1.0f),
      Width(800.0f),
      Height(300.0f),
      Depth(500.0f),
      LeftMaterial(EMetaXRAudioMaterialPreset::GypsumBoard),
      RightMaterial(EMetaXRAudioMaterialPreset::GypsumBoard),
      CeilingMaterial(EMetaXRAudioMaterialPreset::AcousticTile),
      FloorMaterial(EMetaXRAudioMaterialPreset::Carpet),
      FrontMaterial(EMetaXRAudioMaterialPreset::GypsumBoard),
      BackMaterial(EMetaXRAudioMaterialPreset::GypsumBoard) {
  if (ActiveAcoustics) {
    UE_LOG(
        LogAudio,
        Warning,
        TEXT(
            "Meta XR Audio: Multiple Room Acoustics Properties components have been created. This is expected to be a singleton and creating multiple copies in the same level can result in unexpected behavior. The singleton will be set to this new component."));
  }

  ActiveAcoustics = this;

  // If we are using FMOD or Wwise, the room acoustics will not update on a tick like Native Unreal.
  // Therefore, we must apply any editor settings immediately
  FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
}

void UMetaXRAudioRoomAcousticProperties::BeginDestroy() {
  if (ActiveAcoustics == this)
    ActiveAcoustics = nullptr;
  Super::BeginDestroy();
}

void UMetaXRAudioRoomAcousticProperties::UpdateRoomModel(ovrAudioContext Context) {
  float UnitScale = 0.0f;
  ovrResult Result = OVRA_CALL(ovrAudio_GetUnitScale)(Context, &UnitScale);
  check(Result == ovrSuccess);

  ovrAudioAdvancedBoxRoomParameters Params{};
  Params.abrp_Size = sizeof(ovrAudioAdvancedBoxRoomParameters);
  Params.abrp_Width = Width * UnitScale;
  Params.abrp_Height = Height * UnitScale;
  Params.abrp_Depth = Depth * UnitScale;
  Params.abrp_LockToListenerPosition = LockPositionToListener;

  AActor* Parent = GetOwner();
  if (!Parent) {
    return;
  }

  FVector Position = MetaXRAudioUtilities::ToOVRVector(Parent->GetActorLocation());
  Params.abrp_RoomPosition.x = Position.X;
  Params.abrp_RoomPosition.y = Position.Y;
  Params.abrp_RoomPosition.z = Position.Z;

  GetWallMaterialAudioBands(RightMaterial, Params.abrp_ReflectRight);
  GetWallMaterialAudioBands(LeftMaterial, Params.abrp_ReflectLeft);
  GetWallMaterialAudioBands(CeilingMaterial, Params.abrp_ReflectUp);
  GetWallMaterialAudioBands(FloorMaterial, Params.abrp_ReflectDown);
  GetWallMaterialAudioBands(FrontMaterial, Params.abrp_ReflectFront);
  GetWallMaterialAudioBands(BackMaterial, Params.abrp_ReflectBehind);

  // Try to set the room but if the context is unitialized, that is okay, just return for now until it is
  Result = OVRA_CALL(ovrAudio_SetAdvancedBoxRoomParameters)(Context, &Params);
  if (Result == ovrError_AudioUninitialized) {
    // UE_LOG(LogAudio, Warning, TEXT("Meta XR Audio Warning - Context uninitialized during Room Acoustics Call"));
    return;
  }
  check(Result == ovrSuccess);

  ovrAudioBands ClutterBands{};
  float Factor = ClutterFactor;
  for (int32_t Band = OVRA_REVERB_BAND_COUNT - 1; Band >= 0; --Band) {
    ClutterBands[Band] = Factor;
    Factor *= 0.5f;
  }

  Result = OVRA_CALL(ovrAudio_SetRoomClutterFactor)(Context, ClutterBands);
  check(Result == ovrSuccess);
}

UMetaXRAudioRoomAcousticProperties* UMetaXRAudioRoomAcousticProperties::GetActiveRoomAcoustics() {
  return ActiveAcoustics;
}

void UMetaXRAudioRoomAcousticProperties::GetWallMaterialAudioBands(EMetaXRAudioMaterialPreset MaterialPreset, ovrAudioBands Bands) {
  ovrResult Result = ovrAudio_GetReflectionBands(static_cast<ovrAudioMaterialPreset>(MaterialPreset), Bands);
  check(Result == ovrSuccess);
}

#if WITH_EDITOR
void UMetaXRAudioRoomAcousticProperties::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) {
  // The following function will set all the room acoustic properties for External Contexts (FMOD and Wwise)
  // The Contexts for native unreal audio engine are updated on a tick within the audio mixer class.
  // The audio mixer is not compiled into the plugin for External engines and the tick won't be called in that case.
  // If using the native audio engine, this call will do nothing, and so only the tick will apply
  FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// *************************************************************************************************
// Define blueprint setters here
// *************************************************************************************************
void UMetaXRAudioRoomAcousticProperties::SetLockPositionToListener(bool NewLockPositionToListener) {
  if (NewLockPositionToListener != LockPositionToListener) {
    LockPositionToListener = NewLockPositionToListener;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetClutterFactor(float NewClutterFactor) {
  if (NewClutterFactor != ClutterFactor) {
    if (NewClutterFactor < 0.0f || NewClutterFactor > 1.0f) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid clutter factor value %f, must be between 0 and 1."), NewClutterFactor);
      return;
    }
    ClutterFactor = NewClutterFactor;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetWidth(float NewWidth) {
  if (NewWidth != Width) {
    if (NewWidth <= 0.0f) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid width value %f, must be greater than 0."), NewWidth);
      return;
    }
    Width = NewWidth;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetHeight(float NewHeight) {
  if (NewHeight != Height) {
    if (NewHeight <= 0.0f) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid height value %f, must be greater than 0."), NewHeight);
      return;
    }
    Height = NewHeight;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetDepth(float NewDepth) {
  if (NewDepth != Depth) {
    if (NewDepth <= 0.0f) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid depth value %f, must be greater than 0."), NewDepth);
      return;
    }
    Depth = NewDepth;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetLeftMaterial(EMetaXRAudioMaterialPreset NewLeftMaterial) {
  if (NewLeftMaterial != LeftMaterial) {
    LeftMaterial = NewLeftMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetRightMaterial(EMetaXRAudioMaterialPreset NewRightMaterial) {
  if (NewRightMaterial != RightMaterial) {
    RightMaterial = NewRightMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetFloorMaterial(EMetaXRAudioMaterialPreset NewFloorMaterial) {
  if (NewFloorMaterial != FloorMaterial) {
    FloorMaterial = NewFloorMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetCeilingMaterial(EMetaXRAudioMaterialPreset NewCeilingMaterial) {
  if (NewCeilingMaterial != CeilingMaterial) {
    CeilingMaterial = NewCeilingMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetFrontMaterial(EMetaXRAudioMaterialPreset NewFrontMaterial) {
  if (NewFrontMaterial != FrontMaterial) {
    FrontMaterial = NewFrontMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetBackMaterial(EMetaXRAudioMaterialPreset NewBackMaterial) {
  if (NewBackMaterial != BackMaterial) {
    BackMaterial = NewBackMaterial;
    FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
  }
}

void UMetaXRAudioRoomAcousticProperties::SetRoomAcousticProperties(
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
    const EMetaXRAudioMaterialPreset NewBackMaterial) {
  // Set values if they will not be invalid
  if (NewClutterFactor < 0.0f || NewClutterFactor > 1.0f) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid clutter factor value %f, must be between 0 and 1."), NewClutterFactor);
  } else {
    ClutterFactor = NewClutterFactor;
  }

  if (NewWidth <= 0.0f) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid width value %f, must be greater than 0."), NewWidth);
  } else {
    Width = NewWidth;
  }

  if (NewHeight <= 0.0f) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid height value %f, must be greater than 0."), NewHeight);
  } else {
    Height = NewHeight;
  }

  if (NewDepth <= 0.0f) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio - Invalid depth value %f, must be greater than 0."), NewDepth);
  } else {
    Depth = NewDepth;
  }

  // Set the new values for materials. These cannot be invalid because they are enums
  LockPositionToListener = NewLockPositionToListener;
  LeftMaterial = NewLeftMaterial;
  RightMaterial = NewRightMaterial;
  CeilingMaterial = NewCeilingMaterial;
  FloorMaterial = NewFloorMaterial;
  FrontMaterial = NewFrontMaterial;
  BackMaterial = NewBackMaterial;

  // Apply the updates to the context
  FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
}

// *************************************************************************************************
// Define blueprint getters here
// *************************************************************************************************
bool UMetaXRAudioRoomAcousticProperties::GetLockPositionToListener() const {
  return LockPositionToListener;
}

float UMetaXRAudioRoomAcousticProperties::GetClutterFactor() const {
  return ClutterFactor;
}

float UMetaXRAudioRoomAcousticProperties::GetWidth() const {
  return Width;
}

float UMetaXRAudioRoomAcousticProperties::GetHeight() const {
  return Height;
}

float UMetaXRAudioRoomAcousticProperties::GetDepth() const {
  return Depth;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetLeftMaterial() const {
  return LeftMaterial;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetRightMaterial() const {
  return RightMaterial;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetFloorMaterial() const {
  return FloorMaterial;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetCeilingMaterial() const {
  return CeilingMaterial;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetFrontMaterial() const {
  return FrontMaterial;
}

EMetaXRAudioMaterialPreset UMetaXRAudioRoomAcousticProperties::GetBackMaterial() const {
  return BackMaterial;
}
