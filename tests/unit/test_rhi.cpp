#include <catch2/catch_test_macros.hpp>
#include <vyra/rhi/rhi.hpp>

TEST_CASE("VYRA RHI - Context Creation Interface", "[rhi][context]") {
    auto context = vyra::rhi::RHIContext::Create();
    REQUIRE(context != nullptr);

    vyra::rhi::ContextCreateInfo info{};
    info.AppName = "VYRA Unit Test";
    info.EnableValidation = false;

    bool success = context->Init(info);
    REQUIRE(success);
    REQUIRE(context->GetInstance() != nullptr);
    REQUIRE_FALSE(context->IsValidationEnabled());

    REQUIRE_NOTHROW(context->Shutdown());
}

TEST_CASE("VYRA RHI - Device Interface Allocator", "[rhi][device]") {
    auto device = vyra::rhi::RHIDevice::Create();
    REQUIRE(device != nullptr);
}

TEST_CASE("VYRA RHI - SwapChain Interface Allocator", "[rhi][swapchain]") {
    auto swapchain = vyra::rhi::RHISwapChain::Create();
    REQUIRE(swapchain != nullptr);
}
