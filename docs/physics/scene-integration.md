# Physics and Scene Integration

Attach `PhysicsBody2DComponent` to an entity with `Transform2DComponent` and
register a `Physics2DSceneSystem` with the scene. The system synchronizes
transform state into the physics world on each fixed tick, steps the 2D solver,
then writes position and rotation back to the ECS components.

The 3D equivalent uses `PhysicsBody3DComponent`, `TransformComponent` and
`Physics3DSceneSystem`. The 3D rigid-body implementation currently integrates
translation and force/gravity only; angular inertia, collision response and
joint constraints remain future work.

Scene runtime systems receive one `OnStart`, zero or more `OnFixedUpdate`
callbacks per frame, one variable-rate `OnUpdate`, and reverse-order
`OnStop`. Removing a live system calls its stop hook before removal.
