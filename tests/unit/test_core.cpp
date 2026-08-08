#include <catch2/catch_test_macros.hpp>
#include <vyra/core/base.hpp>
#include <vyra/core/log.hpp>
#include <vyra/core/uuid.hpp>
#include <vyra/core/time.hpp>
#include <set>

TEST_CASE("VYRA Core - Logger Initialization", "[core][log]") {
    REQUIRE_NOTHROW(vyra::Log::Init());
    REQUIRE(vyra::Log::GetCoreLogger() != nullptr);
    REQUIRE(vyra::Log::GetClientLogger() != nullptr);
    
    VYRA_LOG_INFO("Core logger test execution.");
    VYRA_INFO("Client logger test execution.");
    
    REQUIRE_NOTHROW(vyra::Log::Shutdown());
}

TEST_CASE("VYRA Core - UUID Uniqueness", "[core][uuid]") {
    std::set<uint64_t> generatedUUIDs;
    constexpr size_t count = 10000;
    
    for (size_t i = 0; i < count; ++i) {
        vyra::UUID uuid;
        REQUIRE(generatedUUIDs.find((uint64_t)uuid) == generatedUUIDs.end());
        generatedUUIDs.insert((uint64_t)uuid);
    }
    
    REQUIRE(generatedUUIDs.size() == count);
}

TEST_CASE("VYRA Core - Time & Timestep", "[core][time]") {
    vyra::Time::Init();
    
    float startTime = vyra::Time::GetTime();
    REQUIRE(startTime >= 0.0f);
    
    vyra::Timestep ts(0.016f); // ~60fps frame
    REQUIRE(ts.GetSeconds() == 0.016f);
    REQUIRE(ts.GetMilliseconds() == 16.0f);
}
