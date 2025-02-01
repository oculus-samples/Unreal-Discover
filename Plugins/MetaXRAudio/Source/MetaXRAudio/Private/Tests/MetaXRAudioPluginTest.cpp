// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MetaXRAudioDllManager.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_DynamicRoom.h"
#include "Misc/AutomationTest.h"


DEFINE_LOG_CATEGORY_STATIC(LogMetaXRAudio, All, All);

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
#include "Tests/AutomationEditorCommon.h"
#endif

// Returns false on fail
#define OVR_AUDIO_TEST(Result, Context)                                                      \
  UE_LOG(LogMetaXRAudio, Log, TEXT("Testing ##Result"));                                     \
  if (Result != ovrSuccess) {                                                                \
    const TCHAR* ErrString = GetMetaXRErrorString(Result);                                   \
    UE_LOG(LogMetaXRAudio, Error, TEXT("Meta XR Audio SDK Unit Test Fail - %s"), ErrString); \
    IsTestSuccessful = false;                                                                \
  }

#define OVR_GETTER_TEST(Variable, ExpectedValue)                             \
  if (Variable != ExpectedValue) {                                           \
    UE_LOG(LogMetaXRAudio, Error, TEXT("##Variable has unexpected value.")); \
    IsTestSuccessful = false;                                                \
  }

/*
 * ------------------ Dependencies test--------------------------
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAllDependenciesTest,
    "MetaXRAudio.DependenciesCheck",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAllDependenciesTest::RunTest(const FString& Parameters) {
  // Windows DLL
  if (!FPaths::FileExists(FPaths::ProjectPluginsDir() + "/MetaXRAudio/Binaries/Win64/metaxraudio64.dll")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find metaxraudio64.dll"));
    return false;
  }

  // Android 64
  if (!FPaths::FileExists(
          FPaths::ProjectPluginsDir() + "/MetaXRAudio/Source/MetaXRAudio/Private/LibMetaXRAudio/lib/arm64-v8a/libmetaxraudio64.so")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find libmetaxraudio64.so"));
    return false;
  }

  // Android 32
  if (!FPaths::FileExists(
          FPaths::ProjectPluginsDir() + "/MetaXRAudio/Source/MetaXRAudio/Private/LibMetaXRAudio/lib/armeabi-v7a/libmetaxraudio32.so")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find libmetaxraudio64.so"));
    return false;
  }

  return true;
}

/*
 * ------------------ API test--------------------------
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCallsTest,
    "MetaXRAudio.ApiCalls",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

static uint32_t MockProgress(void* UserData, const char* String, float Progress) {
  return (uint32_t) true;
};

bool FApiCallsTest::RunTest(const FString& Parameters) {
  bool IsTestSuccessful = true;
  ovrAudioContext Context;

  // Create and initialize context
  {
    ovrAudioContextConfiguration Config;
    Config.acc_Size = sizeof(Config);
    Config.acc_SampleRate = 48000;
    Config.acc_BufferLength = 512;
    Config.acc_MaxNumSources = 16;

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateContext)(&Context, &Config), Context)
  }

  // Feature enable
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_LateReverberation, 1), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_LateReverberation, 0), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_SimpleRoomModeling, 1), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_SimpleRoomModeling, 0), Context);

  // Unit Scale
  {
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetUnitScale)(Context, 0.01f), Context);
    float UnitScale;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_GetUnitScale)(Context, &UnitScale), Context);
    OVR_GETTER_TEST(UnitScale, 0.01f);
  }

  // Box parameters
  {
    ovrAudioAdvancedBoxRoomParameters BoxParameters{};
    BoxParameters.abrp_Size = sizeof(ovrAudioAdvancedBoxRoomParameters);
    BoxParameters.abrp_Depth = 1;
    BoxParameters.abrp_Height = 1;
    BoxParameters.abrp_Width = 1;

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAdvancedBoxRoomParameters)(Context, &BoxParameters), Context);
  }

  // Clutter Factor
  {
    ovrAudioBands ClutterBands = {1.0f, 1.0f, 1.0f, 1.0f};
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetRoomClutterFactor)(Context, ClutterBands), Context);
  }

  // Listener vectors
  {
    float PosX = 0;
    float PosY = 0;
    float PosZ = 0;
    float FwdX = 1;
    float FwdY = 0;
    float FwdZ = 0;
    float UpX = 0;
    float UpY = 0;
    float UpZ = 1;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetListenerVectors)(Context, PosX, PosY, PosZ, FwdX, FwdY, FwdZ, UpX, UpY, UpZ), Context);
  }

  {
    int Sound = 0;
    float PosX = 0;
    float PosY = 0;
    float PosZ = 0;
    float FwdX = 1;
    float FwdY = 0;
    float FwdZ = 0;
    float UpX = 0;
    float UpY = 0;
    float UpZ = 1;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceVectors)(Context, Sound, PosX, PosY, PosZ, FwdX, FwdY, FwdZ, UpX, UpY, UpZ), Context);
  }
  // Source Directivity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSourceDirectivityEnabled)(Context, 0, 1), Context);

  // Source Directivity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSourceDirectivityIntensity)(Context, 0, 1), Context);

  // Audio Source Radius
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceRadius)(Context, 0, 1.0f), Context);

  // HRTF Intensity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioHrtfIntensity)(Context, 0, 1.0f), Context);

  // Reverb Reach
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceReverbReach)(Context, 0, 1.0f), Context);

  // Reflection Send Levels
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioReflectionsSendLevel)(Context, 0, 1.0f), Context);

  // Reverb Send Levels
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioReverbSendLevel)(Context, 0, 1.0f), Context);

  // Audio Source Flags
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceFlags)(Context, 0, ovrAudioSourceFlag_ReflectionsDisabled), Context);

  // Mix-In Shared Reverb
  {
    uint32_t Status = 0;
    float Interleaved[2];
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_MixInSharedReverbInterleaved)(Context, &Status, Interleaved), Context);
  }

  // Shared Reverb Wet Level

  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSharedReverbWetLevel)(Context, 1.0f), Context);

  // Head Radius
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetHeadRadius)(Context, 1.0f), Context);

  // Ambisonics
  {
    ovrAudioAmbisonicStream Stream;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateAmbisonicStream)(Context, 1, 1, ovrAudioAmbisonicFormat_AmbiX, 1, &Stream), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAmbisonicOrientation)(Stream, 1, 0, 0, 0, 0, 1), Context);

    float InData[1];
    float OutData[1];
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_ProcessAmbisonicStreamInterleaved)(Context, Stream, InData, OutData, 1), Context);
  }


  OVRA_CALL(ovrAudio_DestroyContext)(Context);

  return IsTestSuccessful;
}
#endif // WITH_DEV_AUTOMATION_TESTS
