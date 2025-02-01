// Copyright (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#ifndef METAXRAUDIOUTILITIES
#define METAXRAUDIOUTILITIES

#include "MetaXRAudioPlatform.h"
#include "MetaXR_Audio.h"

class MetaXRAudioUtilities {
 public:
  // Helper function to convert from UE coords to OVR coords.
  static FVector ToOVRVector(const FVector& InVec) {
    return FVector(InVec.Y, InVec.Z, -InVec.X);
  }

  // Helper function to convert from OVR coords to UE coords.
  static FVector ToUEVector(const FVector& InVec) {
    return FVector(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector3f ToUEVector3f(const FVector3f& InVec) {
    return FVector3f(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector ToOVRVector(const Audio::FChannelPositionInfo& ChannelPositionInfo) {
    FVector OvrVector;
    OvrVector.X = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Cos(ChannelPositionInfo.Elevation);
    OvrVector.Y = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Sin(ChannelPositionInfo.Elevation);
    OvrVector.Z = ChannelPositionInfo.Radius * FMath::Cos(ChannelPositionInfo.Azimuth);

    return OvrVector;
  }

  static float dbToLinear(float db) {
    return powf(10.0f, db / 20.0f);
  }

  static bool IsMetaXRAudioTheCurrentSpatializationPlugin() {
#ifdef META_NATIVE_UNREAL_PLUGIN
#if WITH_EDITOR
    static FString MetaXRDisplayName = FString(TEXT("Meta XR Audio"));
    return AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
#else // !WITH_EDITOR
    // For non editor situations, we can cache whether this is the current plugin or not the first time we check.
    static FString MetaXRDisplayName = FString(TEXT("Meta XR Audio"));
    static bool bCheckedSpatializationPlugin = false;
    static bool bIsMetaXRCurrentSpatiatizationPlugin = false;

    if (!bCheckedSpatializationPlugin) {
      bIsMetaXRCurrentSpatiatizationPlugin =
          AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
      bCheckedSpatializationPlugin = true;
    }

    return bIsMetaXRCurrentSpatiatizationPlugin;
#endif // WITH_EDITOR
#else // !META_NATIVE_UNREAL_PLUGIN
    return false;
#endif // META_NATIVE_UNREAL_PLUGIN
  }
};

#endif // METAXRAUDIOUTILITIES
