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

Feature - Ethan:
- Rigid body translation: Rigid bodies are simulated translating under a variety of forces: impulse (which only occur over a fixed timestep), periodic (which flip direction every period), and constant (which are always applied). The application of forces onto state (e.g. position, rotation, momentum) are calculated using a simple Euler solver.
- Rigid body rotation: Rigid bodies can also be simulated rotating under a variety of forces, including impulse, periodic, and constant.
- Collision detection: Collisions are detected using axis aligned bounding boxes and a corresponding force is applied to the rigid body. When collisions do occur, the velocity and momentum of both objects are entered into a solver, as well as an energy loss parameter, that determines what the final momentums of the objects should be.
- Object types: The collision detection supports three different types of objects colliding: ground, which is considered as infinite mass and thus cannot be moved, object, which have a fixed mass, and player, which can inflict additional force on an object (for example, kicking).
- Player movement: the player's movement is bound to a collider and rigid body, a user interacts with these components through keybinds. In addition, when an active player is detected, the camera is set to follow the player, supporting "look-around" functionality and zoom in/out.