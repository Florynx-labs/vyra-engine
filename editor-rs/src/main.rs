use gpui::{
    App, Bounds, Context, QuitMode, Render, Window, WindowBounds, WindowOptions, div, prelude::*,
    px, rgb, size,
};
use gpui_platform::application;

const BG: u32 = 0x111411;
const SURFACE: u32 = 0x191e1a;
const SURFACE_ALT: u32 = 0x202722;
const BORDER: u32 = 0x303a32;
const TEXT: u32 = 0xe7ece7;
const MUTED: u32 = 0x8e9a90;
const GREEN: u32 = 0x69b58b;
const GREEN_DARK: u32 = 0x203b2c;

struct VyraEditor;

fn label(value: &'static str, color: u32, size: f32) -> impl IntoElement {
    div().text_size(px(size)).text_color(rgb(color)).child(value)
}

fn panel_title(value: &'static str) -> impl IntoElement {
    div()
        .h(px(38.))
        .px_3()
        .flex()
        .items_center()
        .justify_between()
        .border_b_1()
        .border_color(rgb(BORDER))
        .child(
            div()
                .text_size(px(10.))
                .font_weight(gpui::FontWeight::SEMIBOLD)
                .text_color(rgb(MUTED))
                .child(value),
        )
        .child(label("···", MUTED, 15.))
}

fn scene_row(name: &'static str, kind: &'static str, selected: bool) -> impl IntoElement {
    let background = if selected { GREEN_DARK } else { SURFACE };
    let foreground = if selected { TEXT } else { MUTED };
    div()
        .h(px(30.))
        .px_3()
        .flex()
        .items_center()
        .gap_2()
        .bg(rgb(background))
        .child(label(kind, if selected { GREEN } else { MUTED }, 12.))
        .child(label(name, foreground, 12.))
}

fn property_row(name: &'static str, value: &'static str) -> impl IntoElement {
    div()
        .h(px(30.))
        .px_3()
        .flex()
        .items_center()
        .justify_between()
        .child(label(name, MUTED, 11.))
        .child(
            div()
                .w(px(104.))
                .px_2()
                .py_1()
                .rounded_sm()
                .bg(rgb(SURFACE_ALT))
                .child(label(value, TEXT, 11.)),
        )
}

impl Render for VyraEditor {
    fn render(&mut self, _window: &mut Window, _cx: &mut Context<Self>) -> impl IntoElement {
        div()
            .size_full()
            .flex()
            .flex_col()
            .bg(rgb(BG))
            // Application header: restrained branding and project context.
            .child(
                div()
                    .h(px(48.))
                    .px_4()
                    .flex()
                    .items_center()
                    .justify_between()
                    .border_b_1()
                    .border_color(rgb(BORDER))
                    .child(
                        div()
                            .flex()
                            .items_center()
                            .gap_3()
                            .child(
                                div()
                                    .size(px(25.))
                                    .rounded_md()
                                    .bg(rgb(GREEN_DARK))
                                    .flex()
                                    .items_center()
                                    .justify_center()
                                    .child(label("V", GREEN, 15.)),
                            )
                            .child(
                                div()
                                    .flex()
                                    .flex_col()
                                    .gap_0()
                                    .child(
                                        div()
                                            .text_size(px(12.))
                                            .font_weight(gpui::FontWeight::SEMIBOLD)
                                            .text_color(rgb(TEXT))
                                            .child("VYRA ENGINE"),
                                    )
                                    .child(label("EDITOR  /  EARLY ACCESS", MUTED, 9.)),
                            )
                            .child(label("File    Edit    Assets    Entity    Window    Help", MUTED, 11.)),
                    )
                    .child(
                        div()
                            .flex()
                            .items_center()
                            .gap_3()
                            .child(label("●  EDIT MODE", GREEN, 10.))
                            .child(label("Scene_01.vyra  ▾", TEXT, 11.))
                            .child(label("⌕", MUTED, 18.)),
                    ),
            )
            // Main toolbar.
            .child(
                div()
                    .h(px(44.))
                    .px_3()
                    .flex()
                    .items_center()
                    .justify_between()
                    .bg(rgb(SURFACE))
                    .border_b_1()
                    .border_color(rgb(BORDER))
                    .child(
                        div()
                            .flex()
                            .items_center()
                            .gap_2()
                            .child(label("＋  Create", TEXT, 11.))
                            .child(label("↶", MUTED, 16.))
                            .child(label("↷", MUTED, 16.))
                            .child(label("│", BORDER, 14.))
                            .child(label("↖  Select", GREEN, 11.))
                            .child(label("✥  Move", MUTED, 11.))
                            .child(label("⟳  Rotate", MUTED, 11.))
                            .child(label("▢  Scale", MUTED, 11.)),
                    )
                    .child(
                        div()
                            .flex()
                            .items_center()
                            .gap_2()
                            .child(label("▶  Play", TEXT, 11.))
                            .child(label("Ⅱ  Pause", MUTED, 11.))
                            .child(label("■  Stop", MUTED, 11.))
                            .child(
                                div()
                                    .px_3()
                                    .py_1()
                                    .rounded_sm()
                                    .bg(rgb(GREEN_DARK))
                                    .child(label("BUILD", GREEN, 10.)),
                            ),
                    ),
            )
            // Three-column editor workspace.
            .child(
                div()
                    .flex_1()
                    .flex()
                    .gap(px(1.))
                    .child(
                        div()
                            .w(px(236.))
                            .flex()
                            .flex_col()
                            .bg(rgb(SURFACE))
                            .child(panel_title("SCENE HIERARCHY"))
                            .child(
                                div()
                                    .p_2()
                                    .child(
                                        div()
                                            .h(px(29.))
                                            .px_2()
                                            .flex()
                                            .items_center()
                                            .rounded_sm()
                                            .bg(rgb(SURFACE_ALT))
                                            .child(label("⌕  Search entities...", MUTED, 11.)),
                                    ),
                            )
                            .child(scene_row("Scene_01", "▾", true))
                            .child(scene_row("Directional Light", "☼", false))
                            .child(scene_row("Camera", "◉", false))
                            .child(scene_row("Environment", "◇", false))
                            .child(scene_row("Cube", "▣", false))
                            .child(scene_row("Sphere", "●", false))
                            .child(scene_row("Ground", "▱", false))
                            .child(div().flex_1()),
                    )
                    .child(
                        div()
                            .flex_1()
                            .flex()
                            .flex_col()
                            .gap(px(1.))
                            .child(
                                div()
                                    .flex_1()
                                    .flex()
                                    .flex_col()
                                    .bg(rgb(0x151a16))
                                    .child(
                                        div()
                                            .h(px(36.))
                                            .px_3()
                                            .flex()
                                            .items_center()
                                            .justify_between()
                                            .border_b_1()
                                            .border_color(rgb(BORDER))
                                            .child(label("VIEWPORT", MUTED, 10.))
                                            .child(label("Perspective  ▾     Lit  ▾     Grid  ✓", MUTED, 10.)),
                                    )
                                    .child(
                                        div()
                                            .flex_1()
                                            .flex()
                                            .items_center()
                                            .justify_center()
                                            .child(
                                                div()
                                                    .flex()
                                                    .flex_col()
                                                    .items_center()
                                                    .gap_3()
                                                    .child(
                                                        div()
                                                            .size(px(104.))
                                                            .rounded_lg()
                                                            .border_1()
                                                            .border_color(rgb(0x3b5945))
                                                            .bg(rgb(0x1b2a20))
                                                            .flex()
                                                            .items_center()
                                                            .justify_center()
                                                            .child(
                                                                div()
                                                                    .text_size(px(46.))
                                                                    .text_color(rgb(GREEN))
                                                                    .child("◇"),
                                                            ),
                                                    )
                                                    .child(label("VYRA SCENE VIEW", TEXT, 12.))
                                                    .child(label("Viewport renderer connection pending", MUTED, 10.))
                                                    .child(label("Use the scene hierarchy to inspect entities", MUTED, 10.)),
                                            ),
                                    )
                                    .child(
                                        div()
                                            .h(px(27.))
                                            .px_3()
                                            .flex()
                                            .items_center()
                                            .justify_between()
                                            .border_t_1()
                                            .border_color(rgb(BORDER))
                                            .child(label("World Grid  ·  1.0 m", MUTED, 9.))
                                            .child(label("X 0.0    Y 0.0    Z 0.0", MUTED, 9.)),
                                    ),
                            )
                            .child(
                                div()
                                    .h(px(154.))
                                    .flex()
                                    .flex_col()
                                    .bg(rgb(SURFACE))
                                    .child(panel_title("OUTPUT  /  CONSOLE"))
                                    .child(
                                        div()
                                            .p_3()
                                            .flex()
                                            .flex_col()
                                            .gap_2()
                                            .child(label("20:42:08  [INFO]  VYRA Editor shell initialized", MUTED, 10.))
                                            .child(label("20:42:08  [INFO]  GPUI interface layer ready", GREEN, 10.))
                                            .child(label("20:42:08  [WARN]  Engine bridge not connected yet", 0xc9a66b, 10.))
                                            .child(label("20:42:08  [INFO]  Waiting for native runtime...", MUTED, 10.)),
                                    ),
                            ),
                    )
                    .child(
                        div()
                            .w(px(276.))
                            .flex()
                            .flex_col()
                            .bg(rgb(SURFACE))
                            .child(panel_title("INSPECTOR"))
                            .child(
                                div()
                                    .p_3()
                                    .flex()
                                    .flex_col()
                                    .gap_2()
                                    .child(
                                        div()
                                            .flex()
                                            .items_center()
                                            .gap_2()
                                            .child(label("▣", GREEN, 16.))
                                            .child(
                                                div()
                                                    .flex()
                                                    .flex_col()
                                                    .gap_0()
                                                    .child(
                                                        div()
                                                            .text_size(px(12.))
                                                            .font_weight(gpui::FontWeight::MEDIUM)
                                                            .text_color(rgb(TEXT))
                                                            .child("Cube"),
                                                    )
                                                    .child(label("Entity · 0000-0001", MUTED, 9.)),
                                            ),
                                    )
                                    .child(
                                        div()
                                            .h(px(1.))
                                            .bg(rgb(BORDER)),
                                    )
                                    .child(label("TRANSFORM", GREEN, 10.))
                                    .child(property_row("Position", "0.0, 0.0, 0.0"))
                                    .child(property_row("Rotation", "0.0, 0.0, 0.0"))
                                    .child(property_row("Scale", "1.0, 1.0, 1.0"))
                                    .child(label("MESH RENDERER", GREEN, 10.))
                                    .child(property_row("Mesh", "Cube"))
                                    .child(property_row("Material", "Default"))
                                    .child(label("COMPONENTS", GREEN, 10.))
                                    .child(property_row("Status", "Enabled")),
                            )
                            .child(div().flex_1()),
                    ),
            )
            // Persistent status bar.
            .child(
                div()
                    .h(px(26.))
                    .px_3()
                    .flex()
                    .items_center()
                    .justify_between()
                    .bg(rgb(0x151a16))
                    .border_t_1()
                    .border_color(rgb(BORDER))
                    .child(label("VYRA 0.1.4  ·  EDITOR PROTOTYPE", MUTED, 9.))
                    .child(label("C++23   /   Vulkan 1.4   /   GPUI  ·  UI ONLY", MUTED, 9.)),
            )
    }
}

fn main() {
    application()
        .with_quit_mode(QuitMode::LastWindowClosed)
        .run(|cx: &mut App| {
            let bounds = Bounds::centered(None, size(px(1480.), px(920.)), cx);
            cx.open_window(
                WindowOptions {
                    window_bounds: Some(WindowBounds::Windowed(bounds)),
                    ..Default::default()
                },
                |_, cx| cx.new(|_| VyraEditor),
            )
            .expect("failed to open VYRA Editor window");
            cx.activate(true);
        });
}
