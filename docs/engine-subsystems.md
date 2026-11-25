# Engine Subsystems Expansion

This milestone adds connected runtime foundations across physics, scene
execution, rendering data, animation, audio, scripting and input.

## Physics 2D

- Sequential impulse contacts for axis-aligned box colliders.
- Restitution, Coulomb-style friction and positional correction.
- Trigger enter/exit transitions.
- Distance constraints and deterministic spatial hashing.
- Fixed-step integration and interpolation alpha.

The current contact solver uses AABB colliders; rotated box manifolds, continuous
collision detection, sleeping, warm starting and more joint types are not yet
implemented.

## Physics 3D

- Deterministic sweep-and-prune candidate generation.
- Sphere, AABB, capsule-capsule and OBB-OBB overlap queries.
- Fixed-step translational rigid bodies with AABB impulse response, restitution
  and friction.

The 3D solver currently uses translation-only AABB bodies. Angular inertia,
rotational integration, capsule/OBB rigid-body manifolds, CCD and 3D joints
remain future work.

## ECS and scenes

Scenes now own runtime systems with start, fixed-update, variable-update and
stop lifecycle hooks. Fixed updates are driven by a bounded accumulator.
Physics2DSceneSystem and Physics3DSceneSystem synchronize physics components
with ECS transforms. Scene duplication preserves the new physics and 2D sprite
components.

## Rendering

TextureDescriptor, TextureBinding, PBRMaterial and RenderFeatureSet define
validated CPU-side resource/material data, including metallic/roughness
parameters, texture slots and shadow configuration. Vulkan texture residency,
descriptor allocation, PBR shader passes and actual shadow-map rendering still
belong to the renderer backend and are not claimed as complete by this PR.

## Animation

Skeleton hierarchy validation, keyframe pose sampling, quaternion interpolation,
CPU skinning palette generation and a one-dimensional pose blend tree are
available. Animation graphs, retargeting, GPU skinning and editor tooling remain
future work.

## Audio

The backend-neutral mixer supports mono/stereo PCM clips, multiple voices,
sample-rate conversion, gain/pan, clipping protection and listener-relative
spatial attenuation/panning. It does not yet own an audio device or provide
streaming decoders, reverbs, filters or an effects graph.

## Scripting

The native scripting API defines script lifecycle hooks, a factory registry,
instances and generation-based factory replacement for hot reload. This is not
yet a bytecode VM or embedded language runtime; live instances must be detached
and recreated to pick up a replacement factory.

## Input

Named actions remain in the runtime module. The input module adds controller
axis normalization, button state and configurable deadzones. SDL/gamepad
hardware polling and rebinding UI remain platform/editor adapters.
