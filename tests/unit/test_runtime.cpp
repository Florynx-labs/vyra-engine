#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <vyra/runtime/runtime.hpp>

#include <cmath>
#include <vector>

TEST_CASE("Runtime event bus dispatches typed events and supports removal", "[runtime][events]") {
    struct DamageEvent { int amount; };
    vyra::runtime::EventBus bus;
    int received = 0;
    const auto subscription = bus.Subscribe<DamageEvent>([&](const DamageEvent& event) {
        received += event.amount;
    });

    REQUIRE(bus.GetListenerCount() == 1);
    REQUIRE(bus.Publish(DamageEvent{7}) == 1);
    REQUIRE(received == 7);
    REQUIRE(bus.Unsubscribe(subscription));
    REQUIRE_FALSE(bus.Unsubscribe(subscription));
    REQUIRE(bus.Publish(DamageEvent{3}) == 0);
    REQUIRE(received == 7);
}

TEST_CASE("Runtime event bus allows reentrant publish and unsubscribe", "[runtime][events]") {
    struct TickEvent { int value; };
    vyra::runtime::EventBus bus;
    int calls = 0;
    vyra::runtime::EventBus::Subscription id = 0;
    id = bus.Subscribe<TickEvent>([&](const TickEvent&) {
        ++calls;
        bus.Unsubscribe(id);
    });
    bus.Publish(TickEvent{1});
    bus.Publish(TickEvent{2});
    REQUIRE(calls == 1);
}

TEST_CASE("Input action map exposes down, pressed and released edges", "[runtime][input]") {
    vyra::runtime::InputActionMap input;
    REQUIRE(input.Bind("Jump", 32));
    REQUIRE(input.Bind("Jump", 1001));

    input.SetButtonDown(32, true);
    input.AdvanceFrame();
    REQUIRE(input.IsDown("Jump"));
    REQUIRE(input.WasPressed("Jump"));
    REQUIRE_FALSE(input.WasReleased("Jump"));

    input.AdvanceFrame();
    REQUIRE(input.IsDown("Jump"));
    REQUIRE_FALSE(input.WasPressed("Jump"));

    input.SetButtonDown(32, false);
    input.AdvanceFrame();
    REQUIRE(input.IsDown("Jump")); // alternate binding is still held only if fed
    input.SetButtonDown(1001, true);
    input.AdvanceFrame();
    REQUIRE(input.IsDown("Jump"));

    input.SetButtonDown(1001, false);
    input.AdvanceFrame();
    REQUIRE_FALSE(input.IsDown("Jump"));
    REQUIRE(input.WasReleased("Jump"));
}

TEST_CASE("Fixed step clock bounds catch-up work and reports dropped time", "[runtime][time]") {
    vyra::runtime::FixedStepClock clock(0.01, 3, 0.1);
    double simulated = 0.0;
    const auto result = clock.Advance(0.08, [&](double dt) { simulated += dt; });

    REQUIRE(result.steps == 3);
    REQUIRE(simulated == Catch::Approx(0.03));
    REQUIRE(result.droppedTimeSeconds >= 0.04);
    REQUIRE(result.droppedTimeSeconds < 0.06);
    REQUIRE(result.interpolationAlpha >= 0.0);
    REQUIRE(result.interpolationAlpha < 1.0);
}

TEST_CASE("Fixed step clock ignores invalid deltas", "[runtime][time]") {
    vyra::runtime::FixedStepClock clock;
    int steps = 0;
    auto result = clock.Advance(std::nan(""), [&](double) { ++steps; });
    REQUIRE(result.steps == 0);
    REQUIRE(steps == 0);
}

TEST_CASE("Scalar animation track sorts keys and interpolates", "[runtime][animation]") {
    vyra::runtime::ScalarAnimationTrack track;
    REQUIRE(track.AddKeyframe(1.0, 10.0f));
    REQUIRE(track.AddKeyframe(0.0, 0.0f));
    REQUIRE(track.AddKeyframe(2.0, 20.0f));
    REQUIRE(track.GetKeyframeCount() == 3);
    REQUIRE(track.Sample(0.5) == Catch::Approx(5.0f));
    REQUIRE(track.Sample(0.5, vyra::runtime::Interpolation::Step) == Catch::Approx(0.0f));
    REQUIRE(track.Sample(5.0) == Catch::Approx(20.0f));
    REQUIRE(track.Sample(2.5, vyra::runtime::Interpolation::Linear, true) == Catch::Approx(5.0f));
}

TEST_CASE("Animation player supports pause, looping and completion", "[runtime][animation]") {
    vyra::runtime::AnimationPlayer player;
    player.Play();
    player.Update(0.4, 1.0);
    REQUIRE(player.IsPlaying());
    REQUIRE(player.GetTimeSeconds() == Catch::Approx(0.4));
    player.Pause();
    player.Update(0.2, 1.0);
    REQUIRE(player.GetTimeSeconds() == Catch::Approx(0.4));
    player.Play();
    player.Update(0.8, 1.0);
    REQUIRE_FALSE(player.IsPlaying());
    REQUIRE(player.GetTimeSeconds() == Catch::Approx(1.0));

    player.SetLooping(true);
    player.Play(true);
    player.Update(2.25, 1.0);
    REQUIRE(player.IsPlaying());
    REQUIRE(player.GetTimeSeconds() == Catch::Approx(0.25));
}
