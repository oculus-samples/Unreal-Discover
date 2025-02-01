// Copyright (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAudioRoomAcousticVisualizer.h"

#include "EditorModes.h"
#include "MetaXRAudioRoomAcousticProperties.h"

void FMetaXRAudioRoomAcousticVisualizer::DrawVisualization(
    const UActorComponent* Component,
    const FSceneView* View,
    FPrimitiveDrawInterface* PDI) {
  static const IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos"));
  check(CVar != nullptr);

  const UMetaXRAudioRoomAcousticProperties* RoomAcoustics = Cast<const UMetaXRAudioRoomAcousticProperties>(Component);
  AActor* OwnerActor = Component->GetOwner();

  if (CVar == nullptr || CVar->GetInt() == 0 || !RoomAcoustics || !OwnerActor) {
    return;
  }

  const FMatrix BoxToWorld = OwnerActor->GetTransform().ToMatrixNoScale();
  const FVector Radii(RoomAcoustics->GetDepth() / 2.f, RoomAcoustics->GetWidth() / 2.f, RoomAcoustics->GetHeight() / 2.f);
  const FMaterialRenderProxy* MaterialRenderProxy =
      new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), {1.f, 1.f, 1.f, 0.15f});

  DrawBox(PDI, BoxToWorld, Radii, MaterialRenderProxy, SDPG_World);
}
