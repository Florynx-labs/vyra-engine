#include <catch2/catch_test_macros.hpp>
#include <vyra/animation/skeletal_animation.hpp>
#include <vyra/audio/audio_mixer.hpp>
#include <vyra/input/controller.hpp>
#include <vyra/physics2d/world.hpp>
#include <vyra/physics3d/physics3d.hpp>
#include <vyra/renderer/material.hpp>
#include <vyra/scene/scene.hpp>
#include <vyra/scene/physics_systems.hpp>
#include <vyra/scripting/script_runtime.hpp>

#include <cmath>
#include <memory>
#include <vector>

TEST_CASE("2D solver resolves restitution and reports triggers", "[physics2d][solver]") {
    using namespace vyra::physics2d;
    PhysicsWorldSettings2D settings;
    settings.Gravity = {0.0f, 0.0f};
    settings.FixedTimeStep = 0.01f;
    settings.SolverIterations = 4;
    PhysicsWorld2D world(settings);
    std::vector<RigidBody2D> bodies(2);
    bodies[0].UseGravity = bodies[1].UseGravity = false;
    bodies[0].Position = {-0.4f, 0.0f};
    bodies[1].Position = {0.4f, 0.0f};
    bodies[0].Velocity = {1.0f, 0.0f};
    bodies[1].Velocity = {-1.0f, 0.0f};
    bodies[0].Restitution = bodies[1].Restitution = 1.0f;
    world.Step(0.01f, bodies);
    REQUIRE(bodies[0].Velocity.x < 0.0f);
    REQUIRE(bodies[1].Velocity.x > 0.0f);

    bodies[1].IsTrigger = true;
    bodies[0].Position = bodies[1].Position = {0.0f, 0.0f};
    world.Step(0.01f, bodies);
    REQUIRE(world.GetTriggerEvents().size() == 1);
    REQUIRE(world.GetTriggerEvents()[0].Entered);
    bodies[1].Position = {5.0f, 0.0f};
    world.Step(0.01f, bodies);
    REQUIRE(world.GetTriggerEvents().size() == 1);
    REQUIRE_FALSE(world.GetTriggerEvents()[0].Entered);
}

TEST_CASE("2D distance joints constrain body separation", "[physics2d][joints]") {
    using namespace vyra::physics2d;
    PhysicsWorldSettings2D settings;
    settings.Gravity = {0.0f, 0.0f};
    PhysicsWorld2D world(settings);
    std::vector<RigidBody2D> bodies(2);
    bodies[0].UseGravity = bodies[1].UseGravity = false;
    bodies[0].Position = {0.0f, 0.0f};
    bodies[1].Position = {4.0f, 0.0f};
    world.SetJoints({DistanceJoint2D{0, 1, 2.0f, 1.0f, true}});
    world.Step(1.0f / 60.0f, bodies);
    REQUIRE(std::abs(glm::length(bodies[1].Position - bodies[0].Position) - 2.0f) < 0.01f);
}

TEST_CASE("3D broadphase returns deterministic overlapping candidates", "[physics3d][broadphase]") {
    using namespace vyra::physics3d;
    Broadphase3D broadphase;
    const auto pairs = broadphase.ComputePairs({
        AABB{{0,0,0},{2,2,2}}, AABB{{1,1,1},{3,3,3}},
        AABB{{8,8,8},{9,9,9}}, AABB{{0,0,0},{-1,-1,-1}}
    });
    REQUIRE(pairs.size() == 1);
    REQUIRE((pairs[0] == std::pair<uint32_t,uint32_t>{0,1}));
}

TEST_CASE("3D capsule and oriented box collision queries", "[physics3d][shapes]") {
    using namespace vyra::physics3d;
    Contact contact;
    REQUIRE(Overlap(Capsule{{0,-1,0},{0,1,0},0.5f},
                    Capsule{{0.75f,-1,0},{0.75f,1,0},0.5f}, &contact));
    REQUIRE(contact.Penetration > 0.0f);
    OBB a, b;
    a.HalfExtents = {1,1,1};
    b.Center = {1.5f,0,0};
    b.HalfExtents = {1,1,1};
    REQUIRE(Overlap(a,b,&contact));
    REQUIRE(contact.Penetration > 0.0f);
}

TEST_CASE("3D rigid body world integrates fixed steps", "[physics3d][world]") {
    using namespace vyra::physics3d;
    PhysicsWorldSettings3D settings;
    settings.Gravity = {0,-10,0};
    settings.FixedTimeStep = 0.1f;
    PhysicsWorld3D world(settings);
    std::vector<RigidBody3D> bodies(1);
    REQUIRE(world.Advance(0.1f,bodies) == 1);
    REQUIRE(std::abs(bodies[0].Velocity.y + 1.0f) < 1.0e-4f);

    bodies.resize(2);
    bodies[0].UseGravity = bodies[1].UseGravity = false;
    bodies[0].Position = {-0.4f, 0.0f, 0.0f};
    bodies[1].Position = {0.4f, 0.0f, 0.0f};
    bodies[0].Velocity = {1.0f, 0.0f, 0.0f};
    bodies[1].Velocity = {-1.0f, 0.0f, 0.0f};
    bodies[0].Restitution = bodies[1].Restitution = 1.0f;
    world.Step(0.01f, bodies);
    REQUIRE(bodies[0].Velocity.x < 0.0f);
    REQUIRE(bodies[1].Velocity.x > 0.0f);
}

TEST_CASE("Skeletal animation samples and blends bone poses", "[animation]") {
    using namespace vyra::animation;
    AnimationClip clip;
    clip.FramesPerSecond = 1.0f;
    clip.Frames.resize(2);
    clip.Frames[0].push_back(BonePose{});
    BonePose secondPose;
    secondPose.Translation = {2.0f, 0.0f, 0.0f};
    clip.Frames[1].push_back(secondPose);
    const auto pose = Sample(clip,0.5f,false);
    REQUIRE(pose.size() == 1);
    REQUIRE(std::abs(pose[0].Translation.x - 1.0f) < 1.0e-4f);
    BlendTree1D tree;
    REQUIRE(tree.AddChild(0.0f,clip.Frames[0]));
    REQUIRE(tree.AddChild(1.0f,clip.Frames[1]));
    REQUIRE(std::abs(tree.Evaluate(0.5f)[0].Translation.x - 1.0f) < 1.0e-4f);
    Skeleton skeleton;
    skeleton.Bones = {{"root", -1, glm::mat4(1.0f)}, {"child", 0, glm::mat4(1.0f)}};
    std::vector<BonePose> hierarchyPose(2);
    hierarchyPose[0].Translation = {1.0f, 0.0f, 0.0f};
    hierarchyPose[1].Translation = {0.0f, 2.0f, 0.0f};
    const auto palette = BuildSkinningPalette(skeleton, hierarchyPose);
    REQUIRE(palette.size() == 2);
    REQUIRE(std::abs(palette[1][3].x - 1.0f) < 1.0e-4f);
    REQUIRE(std::abs(palette[1][3].y - 2.0f) < 1.0e-4f);
}

TEST_CASE("Audio mixer produces stereo samples and clamps output", "[audio]") {
    using namespace vyra::audio;
    AudioClip clip{48000,1,{0.5f,-0.5f,2.0f}};
    AudioMixer mixer;
    mixer.AddVoice(Voice{&clip,0.0,2.0f,0.0f,false,true});
    float output[6]{};
    mixer.Mix(output,3);
    REQUIRE(output[0] > 0.0f);
    REQUIRE(output[1] > 0.0f);
    REQUIRE(output[4] == 1.0f);
    REQUIRE(output[5] == 1.0f);
    const auto spatial = AudioMixer::Spatialize(Listener3D{}, Emitter3D{{1,0,0},1.0f,0.0f,10.0f});
    REQUIRE(spatial.Gain > 0.0f);
    REQUIRE(spatial.Pan > 0.0f);
}

TEST_CASE("Script registry hot reloads factories by generation", "[scripting]") {
    struct TestScript final : vyra::scripting::Script {
        explicit TestScript(int* value, int add) : counter(value), amount(add) {}
        void OnUpdate(vyra::scripting::ScriptContext&, float) override { *counter += amount; }
        int* counter; int amount;
    };
    struct Context final : vyra::scripting::ScriptContext {};
    using namespace vyra::scripting;
    int counter = 0;
    ScriptRegistry registry;
    REQUIRE(registry.Register("counter",[&]{return std::make_unique<TestScript>(&counter,1);}));
    ScriptInstance instance;
    Context context;
    REQUIRE(instance.Attach(registry.Instantiate("counter"),context));
    instance.Update(context,0.016f);
    REQUIRE(counter == 1);
    REQUIRE(registry.HotReload("counter",[&]{return std::make_unique<TestScript>(&counter,3);}));
    instance.Detach(context);
    REQUIRE(instance.Attach(registry.Instantiate("counter"),context));
    instance.Update(context,0.016f);
    REQUIRE(counter == 4);
    REQUIRE(registry.GetGeneration("counter") == 2);
}

TEST_CASE("Controller mapping applies configurable deadzones", "[input]") {
    using namespace vyra::input;
    ControllerMapper mapper;
    ControllerState state;
    state.Axes[ControllerAxis::LeftX] = 0.1f;
    REQUIRE(mapper.GetAxis(state,ControllerAxis::LeftX) == 0.0f);
    state.Axes[ControllerAxis::LeftX] = 0.5f;
    REQUIRE(mapper.GetAxis(state,ControllerAxis::LeftX) > 0.0f);
    REQUIRE(mapper.SetDeadzone(ControllerAxis::LeftX,0.4f));
    REQUIRE(mapper.GetAxis(state,ControllerAxis::LeftX) > 0.0f);
}

TEST_CASE("PBR material validates normalized parameters", "[renderer][materials]") {
    vyra::renderer::PBRMaterial material;
    REQUIRE(material.IsValid());
    material.Roughness = 2.0f;
    REQUIRE_FALSE(material.IsValid());
    vyra::renderer::RenderFeatureSet features;
    REQUIRE(features.IsValid());
    features.ShadowMapResolution = 32;
    REQUIRE_FALSE(features.IsValid());
}


TEST_CASE("Scene runtime lifecycle drives physics components through ECS", "[scene][physics2d]") {
    using namespace vyra::scene;
    struct LifecycleProbe final : SceneRuntimeSystem {
        int starts{0}, fixed{0}, updates{0}, stops{0};
        void OnStart(Scene&) override { ++starts; }
        void OnFixedUpdate(Scene&, vyra::Timestep) override { ++fixed; }
        void OnUpdate(Scene&, vyra::Timestep) override { ++updates; }
        void OnStop(Scene&) override { ++stops; }
    };

    Scene scene("Runtime integration");
    auto entity = scene.CreateEntity("Player");
    entity.AddComponent<Transform2DComponent>(glm::vec2(0.0f));
    vyra::physics2d::RigidBody2D body;
    body.UseGravity = false;
    body.Velocity = {1.0f, 0.0f};
    entity.AddComponent<PhysicsBody2DComponent>(body);

    vyra::physics2d::PhysicsWorldSettings2D settings;
    settings.Gravity = {0.0f, 0.0f};
    auto physics = std::make_shared<Physics2DSceneSystem>(settings);
    auto probe = std::make_shared<LifecycleProbe>();
    REQUIRE(scene.AddRuntimeSystem(physics));
    REQUIRE(scene.AddRuntimeSystem(probe));
    scene.OnUpdateRuntime(vyra::Timestep(1.0f / 30.0f));
    REQUIRE(probe->starts == 1);
    REQUIRE(probe->fixed == 2);
    REQUIRE(probe->updates == 1);
    REQUIRE(scene.GetRegistry().Get<Transform2DComponent>(entity.GetID()).Position.x > 0.0f);
    scene.StopRuntime();
    REQUIRE(probe->stops == 1);
}
