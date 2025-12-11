# CSCI1230FinalProject

# How to use the program

[Assimp](https://github.com/assimp/assimp/blob/master/Build.md) must be installed and built.

When the program is run, you can load a scenefile. Our final scenes are scenefiles/light_room.json (with movable zebra player) and scenefiles/light_room_player_detached.json (the same scene with a free camera). Many other scenefiles for testing are available in scenefiles/.

Shadows can be toggled in the GUI. L-system trees must be toggled before a scene is loaded in order for the setting to take effect.

Some scenes attach the camera to the player model

# Player & Camera Controls

WASD - move player/camera forward, left, backward, and right respectively

W+R - run forward (in player mode only)

Drag mouse - change camera angle

E - eat

K - kick

T - trot (in detached mode only)

G - gallop (in detached mode only)

I - primary idle

2 - idle 2

3 - idle 3

# Features & Collaboration

Features - Nathan:
- HDR: unbounded light values tone-mapped back to 0-1 range to allow for more dynamic light sources, improved bloom
- Bloom: bright regions of the screen are blurred and composited back onto the screen texture, creating a glow effect for bright lights
- Post-processing pipeline: framework for applying shaders, with support for HDR and bloom
(features below added for project 7)
- Simple Tone-Mapping: modify color values of the scene texture based on various parameters using a texture applied using the post-processing pipeline
- Scene primitives: made the scene primitives that comprise the store scene

  Collaboration - Nathan:
  - Used ChatGPT to debug color grading shader
 
Features - Patrick:
- Textures with Normal Mapping/Bump Mapping: Allows objects to have textures and uses normal map or height map to compute normals and adds additional surface details.
- L-System: Generate trees based on the string and rules. Trees will show if the L-System is checked in the UI before scene parse.

Collaboration - Patrick:
- Many textures in this project are provided by https://3dtextures.me/, which are licensed as CC0 (https://3dtextures.me/about/).

- A few of the texture images from https://3dtextures.me/ are edited and generated from ChatGPT 5.1:
Prompts:
(Provided ChatGPT with Metal_Corrugated_013_basecolor.png from https://3dtextures.me/)
"can you add soda label on top of this texture file so it can be use as a texture file for a soda can"
"can you make another image, instead of cereal do a tuna label"
"Can you add chicken noodle soup label onto this texture"
(Provided ChatGPT with Geometric_wallpaper_basecolor.png from https://3dtextures.me/)
"Can you edit this texture? add a cookie label"
"can you add the cereal label but onto this texture instead"

Features - Ethan:
- Rigid body translation: Rigid bodies are simulated translating under a variety of forces: impulse (which only occur over a fixed timestep), periodic (which flip direction every period), and constant (which are always applied). The application of forces onto state (e.g. position, rotation, momentum) are calculated using a simple Euler solver.
- Rigid body rotation: Rigid bodies can also be simulated rotating under a variety of forces, including impulse, periodic, and constant.
- Collision detection: Collisions are detected using axis aligned bounding boxes and a corresponding force is applied to the rigid body. When collisions do occur, the velocity and momentum of both objects are entered into a solver, as well as an energy loss parameter, that determines what the final momentums of the objects should be.
- Object types: The collision detection supports three different types of objects colliding: ground, which is considered as infinite mass and thus cannot be moved, object, which have a fixed mass, and player, which can inflict additional force on an object (for example, kicking).
- Player movement: the player's movement is bound to a collider and rigid body, a user interacts with these components through keybinds. In addition, when an active player is detected, the camera is set to follow the player, supporting "look-around" functionality and zoom in/out.

Features - John:
- Shadows are implemented for directional and spot lights. Up to 12 lights in a scene can have shadows enabled. The scene depth is rendered from each shadowing light's perspective into a texture. Then, in the main render pass, the textures are sampled and compared against the distance to each light to determine shadow visibility.
- Fog is computed based on camera-space depth. A linear interpolation function is used to blend the fog color and phong illumination.
- Kinematic skeletons are loaded using assimp. The active model can be specified by changing a field in src/shapes/shape_manager.cpp. Multiple animations baked in the same file are supported. The animations are responsive to player movement and keyboard input.

Collaboration - John:
- Much of the assimp boilerplate code (src/skeletal_animation.h/*) was adapted from a [tutorial](https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation) and the associated [repo](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/src/8.guest/2020/skeletal_animation).
- stb_image is used to load texture images for animated models.
- Animated 3D model sources:
  - [Zebra](https://sketchfab.com/3d-models/zebramotions-2546097d0ea94ba88452ce62c041fb87)
  - [Eagle](https://sketchfab.com/3d-models/eagle-dc34aa716318405d974883908181fb41)
  - [Fish](https://sketchfab.com/3d-models/fish-swimming-657a6aeb04a64a90b9a2f3089d25422e)
  - [Woodpecker](https://sketchfab.com/3d-models/woodpecker-7b6c1f3b93c0489280dc693044acfc1f)
- Static 3D mesh sources
  - [Shopping cart](https://www.cgtrader.com/items/2789009/download-page)
  - [Old rusty car](https://sketchfab.com/3d-models/old-rusty-car-2-544aa41de67b48cf89f8fcc2bb06e8f4)
  - [Turtle](https://www.cgtrader.com/items/2184392/download-page)
  - [Dog](https://free3d.com/3d-model/dog-v2--703191.html)



# Known Bugs
- The player model sometimes clips partially into the ground. Usually, this can be fixed by pressing J repeatedly. Some objects can clip through the ground entirely.
- Textures on the old rusty car are imperfect because the objfilereader does not take smoothing groups into account.
