#include "client/pch.hh"

#include "client/splash.hh"

#include "core/cmdline.hh"
#include "core/constexpr.hh"
#include "core/epoch.hh"
#include "core/resource.hh"

#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/language.hh"
#include "client/texture_gui.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

constexpr static int SPLASH_COUNT = 4;
constexpr static std::size_t DELAY_MICROSECONDS = 2000000;
constexpr static const char* SPLASH_PATH = "textures/gui/client_splash.png";

static resource_ptr<TextureGUI> texture;
static float texture_aspect;
static float texture_alpha;

static std::uint64_t end_time;
static std::string current_text;

static void on_glfw_key(const GlfwKeyEvent& event)
{
    end_time = UINT64_C(0);
}

static void on_glfw_mouse_button(const GlfwMouseButtonEvent& event)
{
    end_time = UINT64_C(0);
}

static void on_glfw_scroll(const GlfwScrollEvent& event)
{
    end_time = UINT64_C(0);
}

void client_splash::init(void)
{
    if(cmdline::contains("nosplash")) {
        texture = nullptr;
        texture_aspect = 0.0f;
        texture_alpha = 0.0f;
        return;
    }

    std::random_device randev;
    std::uniform_int_distribution<int> dist(0, SPLASH_COUNT - 1);

    texture = resource::load<TextureGUI>(SPLASH_PATH, TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);
    texture_aspect = 0.0f;
    texture_alpha = 0.0f;

    if(texture) {
        if(texture->size.x > texture->size.y) {
            texture_aspect = static_cast<float>(texture->size.x) / static_cast<float>(texture->size.y);
        } else {
            texture_aspect = static_cast<float>(texture->size.y) / static_cast<float>(texture->size.x);
        }

        texture_alpha = 1.0f;
    }
}

void client_splash::init_late(void)
{
    if(!texture) {
        // We don't have to waste time
        // rendering the missing client_splash texture
        return;
    }

    end_time = epoch::microseconds() + DELAY_MICROSECONDS;

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<GlfwMouseButtonEvent>().connect<&on_glfw_mouse_button>();
    globals::dispatcher.sink<GlfwScrollEvent>().connect<&on_glfw_scroll>();

    current_text = language::resolve("splash.skip_prompt");

    while(!glfwWindowShouldClose(globals::window)) {
        const std::uint64_t curtime = epoch::microseconds();
        const std::uint64_t remains = end_time - curtime;

        if(curtime >= end_time) {
            break;
        }

        texture_alpha = vx::smoothstep(0.25f, 0.6f, static_cast<float>(remains) / static_cast<float>(DELAY_MICROSECONDS));

        client_splash::render();
    }

    globals::dispatcher.sink<GlfwKeyEvent>().disconnect<&on_glfw_key>();
    globals::dispatcher.sink<GlfwMouseButtonEvent>().disconnect<&on_glfw_mouse_button>();
    globals::dispatcher.sink<GlfwScrollEvent>().disconnect<&on_glfw_scroll>();

    texture = nullptr;
    texture_aspect = 0.0f;
    texture_alpha = 0.0f;
    end_time = UINT64_C(0);
}

void client_splash::render(void)
{
    if(!texture) {
        // We don't have to waste time
        // rendering the missing client_splash texture
        return;
    }

    // The client_splash is rendered outside the main
    // render loop, so we have to manually begin
    // and render both window and ImGui frames
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, globals::width, globals::height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto viewport = ImGui::GetMainViewport();
    auto window_start = ImVec2(0.0f, 0.0f);
    auto window_size = ImVec2(viewport->Size.x, viewport->Size.y);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###client_splash", nullptr, WINDOW_FLAGS)) {
        const float image_width = 0.60f * viewport->Size.x;
        const float image_height = image_width / texture_aspect;
        const ImVec2 image_size = ImVec2(image_width, image_height);

        const float image_x = 0.5f * (viewport->Size.x - image_width);
        const float image_y = 0.5f * (viewport->Size.y - image_height);
        const ImVec2 image_pos = ImVec2(image_x, image_y);

        if(!current_text.empty()) {
            ImGui::PushFont(globals::font_chat);
            ImGui::SetCursorPos(ImVec2(16.0f, 16.0f));
            ImGui::TextDisabled("%s", current_text.c_str());
            ImGui::PopFont();
        }

        const ImVec2 uv_a = ImVec2(0.0f, 0.0f);
        const ImVec2 uv_b = ImVec2(1.0f, 1.0f);
        const ImVec4 tint = ImVec4(1.0f, 1.0f, 1.0f, texture_alpha);

        ImGui::SetCursorPos(image_pos);
        ImGui::ImageWithBg(texture->handle, image_size, uv_a, uv_b, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tint);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(globals::window);

    glfwPollEvents();
}
