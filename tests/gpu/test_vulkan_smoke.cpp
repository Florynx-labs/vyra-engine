#include <catch2/catch_test_macros.hpp>
#include <vyra/rhi/rhi.hpp>
#include <vyra/rhi/rhi_error.hpp>
#include <vyra/core/log.hpp>

// Tag for GPU tests
TEST_CASE("VYRA Vulkan - Context Creation", "[vulkan][gpu][smoke]") {
    vyra::Log::Init();
    
    auto context = vyra::rhi::RHIContext::Create();
    REQUIRE(context != nullptr);

    vyra::rhi::ContextCreateInfo info{};
    info.AppName = "VYRA Vulkan Smoke Test";
    info.EnableValidation = false; // Disable for CI headless testing

    bool success = context->Init(info);
    REQUIRE(success);
    REQUIRE(context->GetInstance() != nullptr);
    REQUIRE_FALSE(context->IsValidationEnabled());

    REQUIRE_NOTHROW(context->Shutdown());
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Vulkan - Command Pool Creation", "[vulkan][gpu][smoke]") {
    vyra::Log::Init();
    
    auto context = vyra::rhi::RHIContext::Create();
    vyra::rhi::ContextCreateInfo info{};
    info.AppName = "VYRA Command Pool Test";
    info.EnableValidation = false;
    
    REQUIRE(context->Init(info));
    
    // Note: Cannot fully test without surface/device, but we can test allocation
    auto commandPool = vyra::rhi::RHICommandPool::Create();
    REQUIRE(commandPool != nullptr);
    
    context->Shutdown();
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Vulkan - Synchronization Objects", "[vulkan][gpu][smoke]") {
    vyra::Log::Init();
    
    auto fence = vyra::rhi::RHIFence::Create();
    REQUIRE(fence != nullptr);
    
    auto semaphore = vyra::rhi::RHISemaphore::Create();
    REQUIRE(semaphore != nullptr);
    
    // Test fence operations (without device - just API validation)
    // These would need actual device initialization in integration tests
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Vulkan - Render Pass Creation", "[vulkan][gpu][smoke]") {
    vyra::Log::Init();
    
    auto renderPass = vyra::rhi::RHIRenderPass::Create();
    REQUIRE(renderPass != nullptr);
    
    // Test render pass creation info structure
    vyra::rhi::RenderPassCreateInfo rpInfo{};
    rpInfo.ColorAttachment.Format = 37; // VK_FORMAT_B8G8R8A8_SRGB
    rpInfo.ColorAttachment.Samples = 1;
    rpInfo.ColorAttachment.LoadOp = 2; // VK_ATTACHMENT_LOAD_OP_CLEAR
    rpInfo.ColorAttachment.StoreOp = 1; // VK_ATTACHMENT_STORE_OP_STORE
    rpInfo.ColorAttachment.InitialLayout = 0; // VK_IMAGE_LAYOUT_UNDEFINED
    rpInfo.ColorAttachment.FinalLayout = 5; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    
    // Note: Full render pass creation requires device integration test
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Vulkan - Error Handling", "[vulkan][gpu][smoke]") {
    vyra::Log::Init();
    
    // Test RHI error codes
    vyra::rhi::RHIResult success = vyra::rhi::RHIResult::Success();
    REQUIRE(success.IsSuccess());
    REQUIRE_FALSE(success.IsError());
    
    vyra::rhi::RHIResult error = vyra::rhi::RHIResult::Error(vyra::rhi::RHIErrorCode::ErrorInvalidParameter, "Test error");
    REQUIRE(error.IsError());
    REQUIRE_FALSE(error.IsSuccess());
    REQUIRE(error.GetCode() == vyra::rhi::RHIErrorCode::ErrorInvalidParameter);
    REQUIRE(error.HasMessage());
    
    // Test Vulkan result conversion
    vyra::rhi::RHIResult vkSuccess = vyra::rhi::RHIResult::FromVkResult(VK_SUCCESS);
    REQUIRE(vkSuccess.IsSuccess());
    
    vyra::rhi::RHIResult vkError = vyra::rhi::RHIResult::FromVkResult(VK_ERROR_OUT_OF_DEVICE_MEMORY, "Test");
    REQUIRE(vkError.IsError());
    REQUIRE(vkError.GetCode() == vyra::rhi::RHIErrorCode::ErrorOutOfDeviceMemory);
    
    vyra::Log::Shutdown();
}