#include "ParticleSystem.hpp"
#include "SimulationConfig.hpp"

#include <raylib.h>
#include <string>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main() {

    SetTargetFPS(60);

    SimulationConfig config{
        .screenWidth = 1000,
        .screenHeight = 700,
        .particleCount = 200,
        .gravity = 500.0f,
        .bounceDamping = 0.85f,
        .airDrag = 0.99f,
        .restitution = 0.9f,
    };

    InitWindow(config.screenWidth, config.screenHeight, "Particle Sandbox");

    ParticleSystem particles{config};
    SimulationConfig &mutableConfig = particles.getConfig();
    bool paused = false;
    bool diagnostics = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }

        bool stepFrame = (IsKeyPressed(KEY_N));

        if (IsKeyPressed(KEY_R)) {
            particles.reset();
        }

        if (IsKeyPressed(KEY_D)) {
            diagnostics = !diagnostics;
        }

        if (IsKeyPressed(KEY_M)) {
            particles.toggleMode();
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            particles.spawn({mouse.x, mouse.y});
        }
        if (!paused || stepFrame) {
            particles.update(dt);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (diagnostics) {
            particles.drawGrid();
            DrawText(
                ("Narrowphase checks: " + std::to_string(particles.getNarrowphaseChecks())).c_str(),
                40, 180, 20, RAYWHITE);
            DrawText(
                ("Actual collisions: " + std::to_string(particles.getActualCollisions())).c_str(),
                40, 210, 20, RAYWHITE);

            GuiSlider({40, 300, 200, 20}, "Gravity", TextFormat("%.0f", mutableConfig.gravity),
                      &mutableConfig.gravity, 0.0f, 2000.0f);
            GuiSlider({40, 350, 200, 20}, "Damping", TextFormat("%.2f", mutableConfig.bounceDamping),
                      &mutableConfig.bounceDamping, 0.0f, 1.0f);
            GuiSlider({40, 400, 200, 20}, "Drag", TextFormat("%.2f", mutableConfig.airDrag),
                      &mutableConfig.airDrag, 0.0f, 1.0f);
            GuiSlider({40, 450, 200, 20}, "Restitution", TextFormat("%.2f", mutableConfig.restitution),
                      &mutableConfig.restitution, 0.0f, 1.0f);
            GuiSlider({40, 500, 200, 20}, "Pressure Radius", TextFormat("%.2f", mutableConfig.pressureRadius),
                      &mutableConfig.pressureRadius, 0.0f, 50.0f);
            GuiSlider({40, 550, 200, 20}, "Pressure Strength", TextFormat("%.2f", mutableConfig.pressureStrength),
                      &mutableConfig.pressureStrength, 0.0f, 5000.0f);
        }

        DrawText("Particle Sandbox", 40, 40, 32, RAYWHITE);
        DrawText(("Particles: " + std::to_string(particles.particleCount())).c_str(), 40, 80, 24,
                 RAYWHITE);
        DrawText("Space: pause | N: step | R: reset | D: diagnostics | M: mode | Left mouse: spawn",
                 40, 120, 20, RAYWHITE);
        DrawText(("FPS: " + std::to_string(GetFPS())).c_str(), config.screenWidth - 100, 40, 20,
                 RAYWHITE);
        particles.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
