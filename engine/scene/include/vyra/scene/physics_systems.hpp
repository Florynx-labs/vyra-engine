#pragma once

#include "vyra/scene/scene.hpp"
#include "vyra/scene/components_2d.hpp"
#include "vyra/physics3d/physics3d.hpp"

namespace vyra::scene {

// Bridges ECS transforms to the standalone 2D solver. Each fixed update copies
// the component state into a compact simulation array, steps it, then writes
// results back to the owning entities.
class VYRA_API Physics2DSceneSystem final : public SceneRuntimeSystem {
public:
    explicit Physics2DSceneSystem(physics2d::PhysicsWorldSettings2D settings = {})
        : m_World(settings) {}

    void OnFixedUpdate(Scene& scene, Timestep timestep) override {
        std::vector<vyra::physics2d::RigidBody2D> bodies;
        std::vector<vyra::ecs::EntityID> entities;
        scene.GetRegistry().Each<Transform2DComponent, PhysicsBody2DComponent>(
            [&](vyra::ecs::EntityID entity, Transform2DComponent& transform, PhysicsBody2DComponent& component) {
                component.Body.Position = transform.Position;
                component.Body.Rotation = transform.Rotation;
                bodies.push_back(component.Body);
                entities.push_back(entity);
            });
        if (bodies.empty()) return;
        m_World.Step(timestep.GetSeconds(), bodies);
        for (std::size_t i = 0; i < bodies.size(); ++i) {
            auto& component = scene.GetRegistry().Get<PhysicsBody2DComponent>(entities[i]);
            auto& transform = scene.GetRegistry().Get<Transform2DComponent>(entities[i]);
            component.Body = bodies[i];
            transform.Position = bodies[i].Position;
            transform.Rotation = bodies[i].Rotation;
        }
    }

    physics2d::PhysicsWorld2D& GetWorld() { return m_World; }
private:
    physics2d::PhysicsWorld2D m_World;
};

class VYRA_API Physics3DSceneSystem final : public SceneRuntimeSystem {
public:
    explicit Physics3DSceneSystem(physics3d::PhysicsWorldSettings3D settings = {})
        : m_World(settings) {}

    void OnFixedUpdate(Scene& scene, Timestep timestep) override {
        std::vector<physics3d::RigidBody3D> bodies;
        std::vector<vyra::ecs::EntityID> entities;
        scene.GetRegistry().Each<TransformComponent, PhysicsBody3DComponent>(
            [&](vyra::ecs::EntityID entity, TransformComponent& transform, PhysicsBody3DComponent& component) {
                component.Body.Position = transform.Translation;
                bodies.push_back(component.Body);
                entities.push_back(entity);
            });
        if (bodies.empty()) return;
        m_World.Step(timestep.GetSeconds(), bodies);
        for (std::size_t i = 0; i < bodies.size(); ++i) {
            auto& component = scene.GetRegistry().Get<PhysicsBody3DComponent>(entities[i]);
            auto& transform = scene.GetRegistry().Get<TransformComponent>(entities[i]);
            component.Body = bodies[i];
            transform.Translation = bodies[i].Position;
        }
    }

    physics3d::PhysicsWorld3D& GetWorld() { return m_World; }
private:
    physics3d::PhysicsWorld3D m_World;
};

} // namespace vyra::scene
