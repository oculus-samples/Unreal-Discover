# ShaderCacheScripts

This repository contains a set of utility scripts designed to facilitate various tasks related to Android device development and Unreal Engine integration. The scripts provide functionality for:

1. Enabling traces on Android devices and viewing them in Unreal Insights.
2. Recording Pipeline State Objects.
3. Building Shader Pipeline Caches (SPCs).

## Resolving Frame Stalls

In some cases, significant frame stalls may occur during tablet app loading. To mitigate this issue, it is recommended to build a shader pipeline cache. For more information on using these scripts, please refer to the detailed help documentation available by running the scripts with the `-Help` parameter.

## Pre-built Shader Pipeline Cache

A default SPC is provided in the [DefaultSPC](./DefaultSPC/) directory. You can use this pre-built cache instead of building one yourself. To utilize the pre-built cache, simply copy it to the `/Build/Android/PipelineCaches` directory before packaging your build.
