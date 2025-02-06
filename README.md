![Discover Banner](./Documentation/Media/banner.png "Discover")

# Discover

Discover is a Mixed Reality (MR) project that demonstrates how to use key MR features and quickly integrate them in your own project.

This codebase is available both as a reference and as a template for MR projects. The [Oculus License](LICENSE) applies to the SDK and supporting material. The MIT License applies to only certain, clearly marked documents. If an individual file does not indicate which license it is subject to, then the Oculus License applies.

The EOS license can be found [here](https://dev.epicgames.com/en-US/licensing).

See the [CONTRIBUTING](./CONTRIBUTING.md) file for how to help out.

This project was built using the [Unreal Engine](https://www.unrealengine.com/) with [Epic Online Services (EOS)](https://dev.epicgames.com/docs/epic-online-services).

You can test out the Unity version of the game on the [Horizon Store - Discover](https://www.meta.com/experiences/7041851792509764/).

## Project Description

In this project you can see how we use Scene API, Interaction SDK, Passthrough, Spatial Anchors and Shared Spatial Anchors.

## How to run the project in Unreal Engine

**NOTE:** This project can only be used with the Meta Quest fork of Unreal Engine. See <a href="#Dependencies">Dependencies</a> for more information.

1. [Configure the project](./Documentation/Configuration.md) with Meta Quest and EOS
2. Make sure you're using  *Unreal Engine 5.5* or newer.
3. Load the MainMenuMap level.
4. To test in Editor you will need to use Quest Link:
    <details>
      <summary><b>Quest Link</b></summary>

    - Enable Quest Link:
        - Put on your headset and navigate to "Quick Settings"; select "Quest Link" (or "Quest Air Link" if using Air Link).
        - Select your desktop from the list and then select, "Launch". This will launch the Quest Link app, allowing you to control your desktop from your headset.
    - With the headset on, select "Desktop" from the control panel in front of you. You should be able to see your desktop in VR!
    - Navigate to Unreal and change the Play Mode to "VR Preview" using the button with 3 stacked dots in UE's main toolbar.
    - The application should launch on your headset automatically
    </details>

## Dependencies

This project requires the Oculus fork of the UE5.5 game engine that you can find [here](https://github.com/Oculus-VR/UnrealEngine/tree/oculus-5.5).<br/>*NOTE:* access to the Oculus fork requires [access to Epic's GitHub](https://www.unrealengine.com/en-US/ue-on-github).

The following is required to test this project within Unreal Engine:

- [The Meta Quest Link app](https://www.meta.com/quest/setup/)

### Oculus Unreal fork

The Oculus Unreal fork will give you the most up to date integration of Oculus features. However, you must build the editor from its source.

1. [Get access to the Unreal source code](https://developer.oculus.com/documentation/unreal/unreal-building-ue4-from-source/#prerequisites)
2. [Clone the `oculus-5.5` branch of the Oculus fork](hhttps://github.com/Oculus-VR/UnrealEngine/tree/oculus-5.5)
3. [Install Visual Studio](https://developer.oculus.com/documentation/unreal/unreal-building-ue4-from-source/#to-download-and-build-unreal-engine)
4. Open a command prompt in the root of the Unreal, then run this command:
```sh
.\GenerateProjectFiles.bat -Game UnrealDiscover -Engine <full path to Unreal-Discover directory>\UnrealDiscover.uproject
```
5. Open the `UnrealDiscover.sln` file that has been generated in the `Unreal-Discover` directory.
6. Set `UnrealDiscover` as the start-up project and `Development Editor` as the configuration.
7. Hit `F5` to build and debug the project (and the engine).

# Getting the code

First, ensure you have Git LFS installed by running this command:

```sh
git lfs install
```

Then, clone this repo using the "Code" button above, or this command:

```sh
git clone https://github.com/oculus-samples/Unreal-Discover.git
```

# Documentation

More information can be found in the [Documentation](./Documentation/) section of this project.

- [Configuration](./Documentation/Configuration.md)
- [EOS Configuration](./Documentation/EOSConfiguration.md)
- [Discover Overview](./Documentation/DiscoverOverview.md)
- [Project Structure](./Documentation/ProjectStructure.md)
- [Shader Cache Scripts](./ShaderCacheScripts/)
