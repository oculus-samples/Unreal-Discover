# Copyright (c) Meta Platforms, Inc. and affiliates.

param (
	[switch]$StartTrace,
	[switch]$RecordPSOs,
	[Alias("ShowHelp", "-help")]
	[switch]$Help
)

$helpMessage = @"
.SYNOPSIS
	This script is a helper for configuring a build installed on the device.
	VERY IMPORTANT NOTE:
	After packaging your build, you must run it at least 1 time BEFORE using this script.
	Unfortunately, at the time of writing this script, the ADB PUSH operation fails to succeed 
	if the build was has not run at least once. This is because the SDCard directory does not 
	exist until first run. IF this is not acceptible, you can easily accomplish everything in this script by using Project Launcher in editor.
	Or feel free to fix it!

	(1) Package the Project -> (2) Run the project at least 1 time -> (3) Run this script with chosen params to configure the startup command line args.

.DESCRIPTION
	Configuring here means adding commandline args for when the build starts.

.PARAMETER StartTrace
	Setting this to true means we will add command line args for enabling Unreal Insights Trace.
	NOTE: we assume that the Trace Server is running on same machine as this script.
	
	https://dev.epicgames.com/community/learning/tutorials/eB9R/unreal-engine-gathering-unreal-insights-traces-on-android

.PARAMETER RecordPSOs
	Setting this to true means the build will output *.shk and *.rec.upipelinecache for setting up a PSO Cache.
	https://dev.epicgames.com/documentation/en-us/unreal-engine/manually-creating-bundled-pso-caches-in-unreal-engine
	This project has enabled PSO caching in cofig files already.
	This script + RecordPSOs param is used in conjunction with BuildPSOCache.ps1 script that should be located in the same folder as this script.
"@

if($Help) {
	Write-Output $helpMessage;
	return 0;
}

if(-not $StartTrace -and -not $RecordPSOs)
{
	Write-Output "IF StartTrace and RecordPSOs flags are not present, there is nothing to do for this script. Exiting...";
	Write-Output $helpMessage;
	return 0;
}

# first get the device count. At time of writing, I do not know of better way to get device count from adb.
$adbDevicesOutput = adb devices;
$adbDeviceOutputLines = $adbDevicesOutput -split "`n";
$deviceCount = 0;
for ($i = 1; $i -lt $adbDeviceOutputLines.Length; $i++)
{
	if ($adbDeviceOutputLines[$i].Contains('device')) {
		$deviceCount++;
	}
}

if($deviceCount -le 0) {
	Write-Output "Must have a connected android device to run this script. We need to push UECommandLine.txt to device.";
	Write-Output "NOTE: You can accomplish the same thing as this script using the Project Launcher in Unreal Editor. Look at args in script to enable trace...";
	return 1;
}

$gamePackageName = "com.showcases.meta.unreal_discover";
$findPackageResult = (adb shell pm list packages | findstr "com.showcases.meta.unreal_discover");
if([string]::IsNullOrEmpty($findPackageResult)) {
	Write-Output "This script configures game packages that are already installed on android device.";
	Write-Output "Install android packaged build onto device before running this script.";
	return 1;
}

$TraceArgs = "";
if($StartTrace)
{
	# Get first IPv4 addr with Adapter in Up Status. Used for routing trace back to this machine where the trace server is running...
	$IPv4Addr = $null;
	$NetIPConfigs = Get-NetIPConfiguration;
	foreach($NetIPConf in $NetIPConfigs)
	{
		if($NetIPConf.NetAdapter.Status -eq "Up") {
			$IPv4Addr = $NetIPConf.IPv4Address.IPAddress;
			break;
		}
	}

	if([string]::IsNullOrEmpty($IPv4Addr)) {
		Write-Output "When running this script with , IF no network adapters have Up status THEN trace will not be enabled on device.";
		return 1;
	}

	$TraceArgs = "-tracehost=$IPv4Addr -trace=Bookmark,Frame,CPU,GPU,LoadTime,File -statnamedevents";
	adb reverse tcp:1980 tcp:1980;
}

$RecordPSOsArg = "";
if($RecordPSOs)
{
	$RecordPSOsArg = "-logPSO";
}

$UECommandLineFilePath = ".\..\UECommandLine.txt";
$UECommandLineArgs = "$RecordPSOsArg $TraceArgs";
$UECommandLineArgs = $UECommandLineArgs.Trim();

Set-Content -Path $UECommandLineFilePath -Value $UECommandLineArgs;

adb push $UECommandLineFilePath "/sdcard/Android/data/$gamePackageName/files/UnrealGame/UnrealDiscover";
