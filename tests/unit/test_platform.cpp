#include <catch2/catch_test_macros.hpp>
#include <vyra/platform/event.hpp>
#include <vyra/platform/window.hpp>

TEST_CASE("VYRA Platform - Event Dispatching", "[platform][event]") {
    vyra::WindowResizeEvent resizeEvent(1920, 1080);
    REQUIRE(resizeEvent.GetWidth() == 1920);
    REQUIRE(resizeEvent.GetHeight() == 1080);
    REQUIRE_FALSE(resizeEvent.Handled);
    REQUIRE(resizeEvent.IsInCategory(vyra::EventCategoryApplication));

    vyra::EventDispatcher dispatcher(resizeEvent);
    bool handled = dispatcher.Dispatch<vyra::WindowResizeEvent>([](vyra::WindowResizeEvent& e) {
        REQUIRE(e.GetWidth() == 1920);
        return true; // Mark as handled
    });

    REQUIRE(handled);
    REQUIRE(resizeEvent.Handled);
}

TEST_CASE("VYRA Platform - Key & Mouse Events", "[platform][event]") {
    vyra::KeyPressedEvent keyPress(vyra::KeyCode::Space, false);
    REQUIRE(keyPress.GetKeyCode() == vyra::KeyCode::Space);
    REQUIRE_FALSE(keyPress.IsRepeat());
    REQUIRE(keyPress.IsInCategory(vyra::EventCategoryKeyboard));
    REQUIRE(keyPress.IsInCategory(vyra::EventCategoryInput));

    vyra::MouseButtonPressedEvent mousePress(vyra::MouseButton::Left);
    REQUIRE(mousePress.GetMouseButton() == vyra::MouseButton::Left);
    REQUIRE(mousePress.IsInCategory(vyra::EventCategoryMouseButton));
    REQUIRE(mousePress.IsInCategory(vyra::EventCategoryMouse));
}

TEST_CASE("VYRA Platform - Window Props Initialization", "[platform][window]") {
    vyra::WindowProps props("Test Engine Window", 1280, 720, true, false);
    REQUIRE(props.Title == "Test Engine Window");
    REQUIRE(props.Width == 1280);
    REQUIRE(props.Height == 720);
    REQUIRE(props.VSync == true);
    REQUIRE(props.Fullscreen == false);
}
