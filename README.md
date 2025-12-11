# CSCI1230FinalProject

# Player Controls

WASD - move player

W+R - run forward

Drag mouse - change camera angle

E - eat

K - kick

T - trot (in detached mode only)

G - gallop (in detached mode only)

I - primary idle

2 - idle 2

3 - idle 3

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
