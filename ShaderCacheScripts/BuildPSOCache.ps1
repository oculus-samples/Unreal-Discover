# Copyright (c) Meta Platforms, Inc. and affiliates.

param (
    [Parameter(Mandatory=$true)]
    [string]$UECmdExecutablePath,
    [Alias("ShowHelp", "H")]
	[switch]$Help
)

$helpMessage = @"
.SYNOPSIS
	This script is a helper for Building the Shader Pipeline Cache.
    Without this, you can experience fairly significant frame during app loading.
    This is meant to ease the process of building the .spc files.
    For more information:
    https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine

    Use ConfigureInstalledBuild.ps1 script to record PSO's.
    You know that process worked when you can see *.rec.upipelinecache files on device here:
    /sdcard/Android/data/com.showcases.meta.unreal_discover/files/UnrealGame/UnrealDiscover/UnrealDiscover/Saved/CollectedPSOs
    
.PARAMETER UECmdExecutablePath
	The path to the UnrealEditor-Cmd executable. 
    NOTE: IF there are issues with running: UnrealEditor-Cmd.exe -run=ShaderPipelineCacheTools.
    A useful debugging trick is to use -waitfordebugger flag. UnrealEditor-Cmd will stop execution. Then go to: Debug -> Attach to Process -> Find UnrealEditor-Cmd.exe and attach.
    This assumes you have symbols for UnrealEditor-Cmd.exe. IF you need this, edit this script with that flag. UnrealEditor-Cmd.exe -waitfordebugger -stdout ....

    Hopefully you never need this...
"@

if($Help)
{
    Write-Output $helpMessage;
    return;
}


$BuiltCacheDestination = ".\..\Build\Android\PipelineCaches";
$BuiltSPCName = "UD0PSO_UnrealDiscover_SF_VULKAN_ES31_ANDROID.spc";
# before building cache, we should check if one exists. We do not want to overwrite the previous cache. Developers responsibility to deal with stale caches...
if(Test-Path "$BuiltCacheDestination\$BuiltSPCName")
{
    $AlreadyBuiltPSOCachePath = Resolve-Path -Path "$BuiltCacheDestination\$BuiltSPCName";
    Write-Output "Built PSO already exists: $AlreadyBuiltPSOCachePath";
    Write-Output "Remove OR Move that cache then re-run this script.";
    return;
}

# check if necessary paths exist
if( !(Test-Path $UECmdExecutablePath) -or !($UECmdExecutablePath.EndsWith("UnrealEditor-Cmd.exe")) )
{
    Write-Output "This script requires the path to UnrealEditor-Cmd.exe";
    return;
}

# collect cooked shader pipeline data
$CookedPipelineCachePath = ".\..\Saved\Cooked\Android_ASTC\UnrealDiscover\Metadata\PipelineCaches";
if( !(Test-Path $CookedPipelineCachePath) )
{
     Write-Output "Unable to find path to cooked pipeline caches.";
     return;
}

$CookedPipelineCacheFilesInfos = (Get-ChildItem -Path $CookedPipelineCachePath);
if($CookedPipelineCacheFilesInfos.Length -le 0) {
    #https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine
    Write-Output "No *.shk or *.upipelinecache to build with. Did you Cook the project before running? There should be at least 1 .shk file in there if you cooked.";
    return;
}

$SHKFileCount = 0;
foreach($FileInfo in $CookedPipelineCacheFilesInfos)
{
    if($FileInfo.Extension -eq '.shk') {
        $SHKFileCount++;
    }
}

if($SHKFileCount -le 0) {
    Write-Output "No *.shk or *.upipelinecache to build with. Did you Cook the project before running? There should be at least 1 .shk file in there if you cooked.";
    return;
}

Write-Output "Shader Key File count: $SHKFileCount";

$TempPSOBuildFolder = ".\temp";
if(Test-Path $TempPSOBuildFolder) {
    # remove previous contents IF they exist
    Remove-Item -Path $TempPSOBuildFolder -Recurse -Force;
}

# create temp dir
New-Item -Path $TempPSOBuildFolder -ItemType Directory;
Copy-Item -Path $CookedPipelineCachePath\* -Destination $TempPSOBuildFolder -Recurse;

# collect the recorded upipelinecache files
adb pull "/sdcard/Android/data/com.showcases.meta.unreal_discover/files/UnrealGame/UnrealDiscover/UnrealDiscover/Saved/CollectedPSOs" $TempPSOBuildFolder;

$RecordedPSOsPath = "$TempPSOBuildFolder\CollectedPSOs";
$CookedPipelineCacheFilesInfos = (Get-ChildItem -Path $RecordedPSOsPath);
if($CookedPipelineCacheFilesInfos.Length -le 0) {
    Write-Output "Unable to build Shader Pipeline Cache because 0 *rec.upipelinecache files detected...";
    return;
}

Write-Output "Recoreded PSOs to Build:";
Write-Output $CookedPipelineCacheFilesInfos;
Move-Item -Path $RecordedPSOsPath\* -Destination $TempPSOBuildFolder;

# get full real paths as executable in a different location.
$TempFolderRealPath = Resolve-Path -Path $TempPSOBuildFolder;
$BuiltSPCPath = "$TempFolderRealPath\$BuiltSPCName";

# For debugging if issues:
# ENGINE_REPO_LOCATION\Engine\Source\Editor\UnrealEd\Private\Commandlets\ShaderPipelineCacheToolsCommandlet.cpp
# use -waitfordebugger if you need to debug... Debug -> Attach to Process -> find UnrealEditor-Cmd.exe

# Build the Shader Pipeline Cache (.spc)
& $UECmdExecutablePath -stdout -UNATTENDED -run=ShaderPipelineCacheTools Expand "$TempFolderRealPath\*.rec.upipelinecache" "$TempFolderRealPath\*.shk" $BuiltSPCPath;


# place the built shader cache into place unreal will use during next cook...
if( !(Test-Path $BuiltCacheDestination) ) {
    New-Item -Path $BuiltCacheDestination -ItemType Directory;
}
Move-Item -Path $BuiltSPCPath -Destination $BuiltCacheDestination;

# remove the temp folder
Remove-Item -Path $TempPSOBuildFolder -Recurse -Force;

# DONE!
$FullPathToSPC = Resolve-Path -Path "$BuiltCacheDestination\$BuiltSPCName";
Write-Output "Built Shader Pipeline Cache successfully. Location: $FullPathToSPC";
Write-Output "IMPORTANT: For the Shader pipeline cache to work, You must Package the project again...";