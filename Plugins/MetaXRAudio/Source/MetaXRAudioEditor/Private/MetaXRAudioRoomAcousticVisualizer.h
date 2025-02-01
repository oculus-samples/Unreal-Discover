// Copyright (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "ComponentVisualizer.h"
#include "CoreMinimal.h"

/**
 *	Editor visualizatino of Room Acoustic Properties box
 */
class METAXRAUDIOEDITOR_API FMetaXRAudioRoomAcousticVisualizer : public FComponentVisualizer {
 public:
  virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
