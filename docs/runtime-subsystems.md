# Runtime Subsystems

The `vyra::runtime` module contains backend-neutral gameplay primitives. It is
header-only and depends only on `vyra::core`, so the editor, game runtime and
tests can share it without importing SDL or Vulkan.

## Typed event bus

`EventBus` routes events by C++ type. Subscriptions return stable tokens that
can be removed explicitly. Publishing snapshots listeners under the mutex and
invokes callbacks after releasing it, allowing nested publication and
self-unsubscription without deadlocking. A listener already copied into an
in-flight publication may still run once after it is unsubscribed.

## Input actions

`InputActionMap` separates gameplay actions from physical key/button codes.
The platform layer feeds button state and calls `AdvanceFrame()` once per
polling frame. Consumers can query held, pressed-edge and released-edge state.
SDL key mapping remains the responsibility of the platform adapter.

## Fixed-step clock

`FixedStepClock` accumulates variable frame deltas and invokes a bounded number
of fixed simulation steps. Invalid deltas are ignored, long frames are clamped,
and excess accumulated time is reported as dropped time to avoid a spiral of
death. The returned interpolation alpha is intended for render-side blending.

## Animation primitives

`ScalarAnimationTrack` stores sorted scalar keyframes and supports step or
linear interpolation, endpoint clamping and optional looping. `AnimationPlayer`
tracks playback time, pause/stop state and loop completion. These primitives do
not yet provide skeletal rigs, blend trees, animation graphs or GPU skinning.

## Scope boundaries

These systems are intentionally independent of SDL, Vulkan, scene components and
a specific audio backend. Backend adapters and ECS/scene integration should be
added in later milestones rather than leaking platform dependencies into the
runtime API.
