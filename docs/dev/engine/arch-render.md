# Rendering

The client renders through OpenGL 3.3 core profile, with no optional extensions required. SDL3 owns the window and the input events; the `video` subsystem sets up both. Only the client links against this code - the server has no renderer.

## Window and video mode

The `video` subsystem creates the window and resolves the video mode. At startup, it picks a display (the primary display, or the one the `-monitor` command-line option names), and it lists the fullscreen modes that display supports. The engine stores the chosen mode - windowed or a specific fullscreen resolution and rate - in the client config, so it persists between runs. Vertical sync is a separate config setting, applied through the window's swap interval.

The `head` subsystem creates the actual OpenGL context on top of the window `video` made. This split exists because the context, and everything that depends on it (shaders, framebuffers, ImGui), can be destroyed and rebuilt without recreating the window itself.

## Shader programs and variants

A `ShaderProgram` loads a vertex and a fragment source file, named by an [identifier](arch-core.md), and compiles them into one OpenGL program. Each source file can declare numbered variant macros with a `#pragma variant N NAME` line; the shader compiler turns each declared variant into a `#define`, with the value set at runtime.

A `ShaderProgram` only recompiles when a variant value actually changes. This lazy update avoids recompiling shaders every frame for state that changes rarely, such as the active fog model.

## The frame pipeline

The client renders each frame at an internal, pixel-scaled resolution, then upscales the result to the window size. The pixel scale is a config setting; it is what gives the game its blocky, low-resolution look regardless of the window's real size.

A frame renders into three separate layers, each with its own color and depth target:

- **Diffuse** - opaque world geometry and the game's own UI elements (crosshair, block outlines). This layer writes both color and depth.
- **Alpha** - translucent world geometry, such as leaves and glass.
- **Fluid** - fluid surfaces, such as water and lava.

The alpha and fluid layers each reuse the diffuse layer's depth buffer for depth testing, copied in before each layer renders. This way, translucent and fluid geometry test correctly against opaque geometry, but write into their own separate color and depth targets instead of overwriting it.

A composite shader then draws one full-screen triangle that reads all three layers' color and depth textures and blends them into a single image. The client blits that image onto the real window-sized backbuffer, using nearest-neighbor filtering to keep the pixelated look.

ImGui renders last, straight onto the default framebuffer, so its UI always draws on top of the game world. The client then presents the frame with a window buffer swap.

## Outlines

The `outline` subsystem draws debug and selection outlines, such as the wireframe box around the block the player is looking at. It uses its own small `ShaderProgram` and a fixed pair of cube and line vertex buffers, and it renders as part of the diffuse layer, before the translucent layers begin.

> **TODO:** add screenshots once the outline shapes are finalized.
