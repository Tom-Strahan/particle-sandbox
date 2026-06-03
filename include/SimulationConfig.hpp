#pragma once

enum class SimulationMode {
    Balls,
    Pressure
};

struct SimulationConfig {
    int screenWidth = 1000;
    int screenHeight = 700;
    int particleCount = 200;

    float gravity = 500.0f;
    float bounceDamping = 0.85f;
    float airDrag = 0.999f;
    float restitution = 0.9f;
    int gridSize = 32;

    float pressureRadius = 28.0f;
    float pressureStrength = 900.0f;

    float smoothingRadius = 32.0f;
    float targetDensity = 8.0f;
    
    SimulationMode mode = SimulationMode::Balls;
};
