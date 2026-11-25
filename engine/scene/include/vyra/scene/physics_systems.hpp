#pragma once

#include "vyra/scene/scene.hpp"
#include "vyra/scene/components_2d.hpp"
#include "vyra/physics3d/physics3d.hpp"

#include <unordered_map>

namespace vyra::scene {

struct SceneTriggerEvent2D {
    vyra::ecs::EntityID First{vyra::ecs::NullEntity};
    vyra::ecs::EntityID Second{vyra::ecs::NullEntity};
    bool Entered{true};
};

struct SceneDistanceJoint2D {
    vyra::ecs::EntityID First{vyra::ecs::NullEntity};
    vyra::ecs::EntityID Second{vyra::ecs::NullEntity};
    float RestLength{1.0f};
    float Stiffness{1.0f};
    bool Enabled{true};
};

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
        m_LastEntityOrder = entities;
        std::unordered_map<vyra::ecs::EntityID, std::uint32_t> entityIndices;
        entityIndices.reserve(entities.size());
        for (std::size_t i = 0; i < entities.size(); ++i)
            entityIndices.emplace(entities[i], static_cast<std::uint32_t>(i));
        std::vector<vyra::physics2d::DistanceJoint2D> joints;
        for (const auto& joint : m_Joints) {
            const auto first = entityIndices.find(joint.First);
            const auto second = entityIndices.find(joint.Second);
            if (first != entityIndices.end() && second != entityIndices.end())
                joints.push_back({first->second, second->second, joint.RestLength, joint.Stiffness, joint.Enabled});
        }
        m_World.SetJoints(std::move(joints));
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
    void SetDistanceJoints(std::vector<SceneDistanceJoint2D> joints) { m_Joints = std::move(joints); }
    std::vector<SceneTriggerEvent2D> GetTriggerEvents() const {
        std::vector<SceneTriggerEvent2D> events;
        for (const auto& event : m_World.GetTriggerEvents()) {
            if (event.FirstBody < m_LastEntityOrder.size() && event.SecondBody < m_LastEntityOrder.size())
                events.push_back({m_LastEntityOrder[event.FirstBody], m_LastEntityOrder[event.SecondBody], event.Entered});
        }
        return events;
    }
private:
    physics2d::PhysicsWorld2D m_World;
    std::vector<SceneDistanceJoint2D> m_Joints;
    std::vector<vyra::ecs::EntityID> m_LastEntityOrder;
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
