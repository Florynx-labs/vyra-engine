# VYRA Physics Modules

This document describes the current physics foundations. Both modules are independent of the editor and can be used by headless runtime code.

## 2D: vyra::physics2d

### Geometry
- Validated axis-aligned boxes and circles.
- AABB/AABB, circle/circle and circle/AABB overlap tests.
- Contact normal, penetration depth and approximate contact point.
- Bounded raycasts against boxes and circles.

### Broadphase
SpatialHash2D maps world-space AABBs to a deterministic uniform grid. ComputePairs() returns sorted, unique candidate index pairs. Invalid bounds and bodies that exceed the configured per-body cell budget are ignored to prevent pathological allocations.

The broadphase returns candidates, not confirmed collisions. Call the narrow-phase Overlap functions for exact supported-shape tests.

### Runtime integration
PhysicsWorld2D advances a vector of RigidBody2D values using a fixed timestep and bounded substeps. It uses semi-implicit Euler integration, gravity, accumulated force, impulses, angular velocity and rational damping. A zero mass denotes a static body.

This first runtime layer does not yet solve contacts, joints, sleeping, continuous collision detection, or scene/ECS component synchronization. Those are explicit follow-up milestones rather than implied capabilities.

## 3D: vyra::physics3d

### Geometry
- Validated axis-aligned boxes and spheres.
- AABB/AABB, sphere/sphere and sphere/AABB overlap tests.
- Contact normal, penetration depth and approximate contact point.
- Bounded raycasts against boxes and spheres.

The current 3D module is a narrow-phase geometry foundation. It does not yet provide a 3D rigid-body world, broadphase, oriented boxes, capsules, mesh collision, or constraint solving.

## Coordinate and numeric conventions

- Distances and positions use engine world units.
- Ray directions must be normalized within a tolerance of 1e-3.
- Negative radii, negative masses, non-finite values, inverted bounds and invalid timestep settings are rejected.
- Contact normals point from the first shape toward the second shape.
- Physics stepping should be driven by a fixed simulation timestep; render interpolation is exposed separately.

## Tests

The Catch2 unit suite includes geometry, tangency, containment, invalid input, broadphase pair de-duplication, fixed-step integration and ray distance tests. Run the full suite using:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DVYRA_BUILD_GPU_TESTS=OFF
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Planned work

1. 2D narrow-phase contact solver, restitution, friction and trigger events.
2. 2D scene/ECS component integration and collision filtering.
3. 3D broadphase, oriented boxes, capsules and convex queries.
4. 3D rigid-body solver, constraints, CCD and sleeping.
5. Physics debug draw, profiling and deterministic replay tests.
