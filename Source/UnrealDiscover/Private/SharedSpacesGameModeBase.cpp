// Copyright (c) Meta Platforms, Inc. and affiliates.


#include "SharedSpacesGameModeBase.h"

AActor* ASharedSpacesGameModeBase::GetControllerStartSpot(AController* Controller)
{
	if (Controller)
	{
		return Controller->StartSpot.Get();
	}

	return nullptr;
}
