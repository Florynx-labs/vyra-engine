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
    std::set<std::string> generatedUUIDs;
    constexpr size_t count = 10000;
    
    for (size_t i = 0; i < count; ++i) {
        vyra::UUID uuid;
        std::string uuidStr = uuid.ToString();
        REQUIRE(generatedUUIDs.find(uuidStr) == generatedUUIDs.end());
        generatedUUIDs.insert(uuidStr);
    }
    
    REQUIRE(generatedUUIDs.size() == count);
}

TEST_CASE("VYRA Core - UUID String Roundtrip", "[core][uuid]") {
    vyra::UUID uuid1;
    std::string uuidStr = uuid1.ToString();
    
    // String should be 36 characters (32 hex + 4 hyphens)
    REQUIRE(uuidStr.length() == 36);
    
    // Should contain hyphens at correct positions
    REQUIRE(uuidStr[8] == '-');
    REQUIRE(uuidStr[13] == '-');
    REQUIRE(uuidStr[18] == '-');
    REQUIRE(uuidStr[23] == '-');
    
    // Roundtrip should preserve UUID
    vyra::UUID uuid2 = vyra::UUID::FromString(uuidStr);
    REQUIRE(uuid1 == uuid2);
}

TEST_CASE("VYRA Core - UUID Version 4 Format", "[core][uuid]") {
    vyra::UUID uuid;
    const uint8_t* data = uuid.GetData();
    
    // Version 4: bits 4-7 of byte 6 should be 0100 (0x40)
    REQUIRE((data[6] & 0xF0) == 0x40);
    
    // Variant 1: bits 6-7 of byte 8 should be 10 (0x80)
    REQUIRE((data[8] & 0xC0) == 0x80);
}

TEST_CASE("VYRA Core - UUID Nil Detection", "[core][uuid]") {
    vyra::UUID uuid1; // Random UUID
    REQUIRE_FALSE(uuid1.IsNil());
    
    uint8_t nilData[16] = {0};
    vyra::UUID nilUUID(nilData);
    REQUIRE(nilUUID.IsNil());
}

TEST_CASE("VYRA Core - Time & Timestep", "[core][time]") {
    vyra::Time::Init();
    
    float startTime = vyra::Time::GetTime();
    REQUIRE(startTime >= 0.0f);
    
    vyra::Timestep ts(0.016f); // ~60fps frame
    REQUIRE(ts.GetSeconds() == 0.016f);
    REQUIRE(ts.GetMilliseconds() == 16.0f);
}
