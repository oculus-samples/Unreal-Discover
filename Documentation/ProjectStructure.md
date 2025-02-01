# Project Structure

Here you will be able to get an high overview of the project structure and the key elements.

## Discover Content

We separated the project per type of Content, keeping it simple to navigate. As an exception to this, we separated the internal applications (DroneRage and MRBike) into their own structure, which we address later on. The directories are self explanatory but we will go over some of them that might be of interest to you.

### Maps

The main scene is MainMenuMap, this is the entry point to the project and the main level to run the Discover Application.

In the [Examples](../Content/Discover/Maps/Examples) folder we can find levels that demonstrates certain features presented in the application. They are simplified levels to more easily understand certain concepts.

* [ColocationMap](../Content/Discover/Maps/Examples/ColocationMap.umap): Demonstrate a simple scene that setup colocation between players in the same room, where they will see scene elements in the same location.
* [RoomMappingMap](../Content/Discover/Maps/Examples/RoomMappingMap.umap): Demonstrates how the Scene API loads the rooms and generate all elements related to the room once the room is mapped by the user in headset.
* [SimpleMRSetupMap](../Content/Discover/Maps/Examples/SimpleMRSetupMap.umap): As the name depict, it's a scene that is showing the setup required to use passthrough and start an MR application.
* [StartupExampleMap](../Content/Discover/Maps/Examples/StartupExampleMap.umap): Simple launcher for the application that handles entitlement checks and logged in user data.

## Discover Blueprints

Here we will give some more information on the core blueprints in Discover.

### BP_DiscoverPawn

This is the player pawn used throughout the demo. It contains all the logic to interact with the scene via poke, point or grab interactions. Additionally, this blueprint replicates the user's headset and controllers' locations to other networked players.

### BP_ColocationComponent

The host colocation space is created with this component, and any previously saved icons are placed in the scene with their stored anchors. It can also align a pawn to an anchor to enable networked colocation.

### BP_PlayerState and BP_GameState

The majority of replicated events are inside these blueprints. These are used to launch and close apps for all players, synchronize 3D icon and pawn placement in the level, and share the scene information with clients.

### BP_OculusXRSceneActor

An actor responsible for creating a scene for the user. For the host, it will query and spawn all scene elements (walls, floor, ceiling). When a client joins the session, it serializes the scene to be sent from the host using one of BP_PlayerState's RPC events, and spawns it in the client's world.

### BP_FakeXRSceneActor

When using the editor it can be faster to load directly in editor without using Link and using the XRSimulator. To enable this feature we created a Fake Room system that will load the equivalent of the Scene API to enable testing in editor.

[FakeSceneMap](../Content/Discover/Maps/Examples/FakeSceneMap.umap) contains an instance of BP_FakeXRSceneActor that can be treated just like a regular OculusXRSceneActor. Child actors are added to this blueprint to represent the walls, floor and ceiling of a generic scene.

## Applications

### DroneRage

Located at Content/DroneRage, this is where all the content of the application is contained. The structure of the content is setup relative to the different element of the game rather than separated by type of assets.

### MRBike

Located at Content/MRBike, this is where all the content of the application is contained.

This is a standalone application that we integrated in Discover. The root of the application is the BikeActor blueprint.
