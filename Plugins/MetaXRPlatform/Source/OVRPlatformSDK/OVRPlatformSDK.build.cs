/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using UnrealBuildTool;
using System.IO;

public class OVRPlatformSDK : ModuleRules
{
    public OVRPlatformSDK(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string LibOVRPlatformDir = "$(ModuleDir)/LibOVRPlatform/";

        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(LibOVRPlatformDir + "lib/LibOVRPlatformImpl64_1.lib");
            PublicDelayLoadDLLs.Add("LibOVRPlatformImpl64_1.dll");
            isLibrarySupported = true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicAdditionalLibraries.Add(LibOVRPlatformDir + "lib/arm64-v8a/libovrplatformloader.so");
            string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "LibOVRPlatform_APL.xml"));
            isLibrarySupported = true;
        }

        if (isLibrarySupported)
        {
            PublicIncludePaths.Add(Path.Combine(LibOVRPlatformDir, "include"));
        }
        else
        {
            System.Console.WriteLine("Oculus Platform plugin not supported for this platform");
        }

    }
}
