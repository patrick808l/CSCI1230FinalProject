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
