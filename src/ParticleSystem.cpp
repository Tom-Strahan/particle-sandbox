#include "ParticleSystem.hpp"
#include "SimulationConfig.hpp"
#include "Vec2.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <raylib.h>

ParticleSystem::ParticleSystem(const SimulationConfig &config) : config(config) { reset(); }

void ParticleSystem::integrate(float dt) {
    const Vec2 gravity{0.0f, config.gravity};

    for (Particle &particle : particles) {
        particle.velocity = particle.velocity * config.airDrag;
        particle.velocity += gravity * dt;
        particle.position += particle.velocity * dt;
    }
}

void ParticleSystem::resolveWallCollisions() {
    for (Particle &particle : particles) {
        if (particle.position.x - particle.radius < 0.0f) {
            particle.position.x = particle.radius;
            particle.velocity.x *= -config.bounceDamping;
        }

        if (particle.position.x + particle.radius > config.screenWidth) {
            particle.position.x = config.screenWidth - particle.radius;
            particle.velocity.x *= -config.bounceDamping;
        }

        if (particle.position.y - particle.radius < 0.0f) {
            particle.position.y = particle.radius;
            particle.velocity.y *= -config.bounceDamping;
        }

        if (particle.position.y + particle.radius > config.screenHeight) {
            particle.position.y = config.screenHeight - particle.radius;
            particle.velocity.y *= -config.bounceDamping;
        }
    }
}

const char *ParticleSystem::modeName() const {
    if (config.mode == SimulationMode::Balls) {
        return "Balls";
    } else {
        return "Pressure";
    }
}

SimulationConfig &ParticleSystem::getConfig() { return config; }

void ParticleSystem::toggleMode() {
    if (config.mode == SimulationMode::Balls) {
        config.mode = SimulationMode::Pressure;
    } else {
        config.mode = SimulationMode::Balls;
    }
}

void ParticleSystem::buildGrid() {
    for (std::vector<size_t> &cell : grid) {
        cell.clear();
    }

    for (size_t i = 0; i < particles.size(); i++) {
        Particle &particle = particles[i];
        int gridX = static_cast<int>(particle.position.x / cellSize);
        int gridY = static_cast<int>(particle.position.y / cellSize);
        gridX = std::clamp(gridX, 0, gridColumns - 1);
        gridY = std::clamp(gridY, 0, gridRows - 1);
        int idx = gridY * gridColumns + gridX;
        grid[idx].push_back(i);
    }
}

void ParticleSystem::resolveBallCollisions() {
    for (int cellY = 0; cellY < gridRows; cellY++) {
        for (int cellX = 0; cellX < gridColumns; cellX++) {
            int cellIndex = cellY * gridColumns + cellX;
            const std::vector<size_t> &currentCell = grid[cellIndex];
            // Check neighbours
            for (int neighbourY = cellY - 1; neighbourY <= cellY + 1; neighbourY++) {
                for (int neighbourX = cellX - 1; neighbourX <= cellX + 1; neighbourX++) {
                    if (neighbourX < 0 || neighbourX >= gridColumns || neighbourY < 0 ||
                        neighbourY >= gridRows) {
                        continue;
                    }
                    int neighbourIndex = neighbourY * gridColumns + neighbourX;
                    const std::vector<size_t> &neighbourCell = grid[neighbourIndex];

                    // Compare particles from current and neighbour cell
                    for (size_t aIndex : currentCell) {
                        for (size_t bIndex : neighbourCell) {
                            if (bIndex <= aIndex) {
                                continue;
                            }

                            Particle &a = particles[aIndex];
                            Particle &b = particles[bIndex];

                            resolveParticlePair(a, b);
                        }
                    }
                }
            }
        }
    }
}

void ParticleSystem::resolveParticlePair(Particle &a, Particle &b) {
    Vec2 delta = b.position - a.position;

    float distance = delta.length();
    float minDistance = a.radius + b.radius;

    narrowphaseChecks++;
    if (distance >= minDistance || distance <= 0.0f) {
        return;
    }
    actualCollisions++;
    Vec2 normal = delta.normalized();
    float overlap = minDistance - distance;

    // Separate overlapping particles.
    a.position += normal * (-overlap * 0.5f);
    b.position += normal * (overlap * 0.5f);

    // Bounce off eachother
    Vec2 relativeVelocity = b.velocity - a.velocity;
    float velocityAlongNormal = relativeVelocity.dot(normal);

    if (velocityAlongNormal < 0.0f) {
        float restitution = config.restitution;

        float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal / 2.0f;
        Vec2 impulse = normal * impulseMagnitude;

        a.velocity += impulse * -1.0f;
        b.velocity += impulse;
    }
}

void ParticleSystem::applyPressureRepulsion(float dt) {
    for (int cellY = 0; cellY < gridRows; cellY++) {
        for (int cellX = 0; cellX < gridColumns; cellX++) {
            int cellIndex = cellY * gridColumns + cellX;
            const std::vector<size_t> &currentCell = grid[cellIndex];

            for (int neighbourY = cellY - 1; neighbourY <= cellY + 1; neighbourY++) {
                for (int neighbourX = cellX - 1; neighbourX <= cellX + 1; neighbourX++) {
                    if (neighbourY < 0 || neighbourY >= gridRows || neighbourX < 0 ||
                        neighbourX >= gridColumns) {
                        continue;
                    }

                    int neighbourIndex = neighbourY * gridColumns + neighbourX;
                    const std::vector<size_t> &neighborCell = grid[neighbourIndex];

                    for (size_t aIndex : currentCell) {
                        for (size_t bIndex : neighborCell) {
                            if (bIndex <= aIndex) {
                                continue;
                            }

                            Particle &a = particles[aIndex];
                            Particle &b = particles[bIndex];

                            Vec2 delta = b.position - a.position;
                            float distance = delta.length();
                            narrowphaseChecks++;

                            if (distance <= 0.0f || distance >= config.pressureRadius) {
                                continue;
                            }

                            actualCollisions++;

                            Vec2 normal = delta / distance;

                            float t = 1.0f - (distance / config.pressureRadius);
                            float forceMagnitude = config.pressureStrength * t;

                            Vec2 force = normal * forceMagnitude;

                            a.velocity += force * (-dt);
                            b.velocity += force * dt;
                        }
                    }
                }
            }
        }
    }
}

void ParticleSystem::computeDensities() {
    for (Particle& particle: particles) {
        particle.density = 0;
    }
    for (int cellY = 0; cellY < gridRows; cellY++) {
        for (int cellX = 0; cellX < gridColumns; cellX++) {
            int cellIndex = cellY * gridColumns + cellX;
            const std::vector<size_t> &currentCell = grid[cellIndex];

            for (int neighbourY = cellY - 1; neighbourY <= cellY + 1; neighbourY++) {
                for (int neighbourX = cellX - 1; neighbourX <= cellX + 1; neighbourX++) {
                    if (neighbourY < 0 || neighbourY >= gridRows || neighbourX < 0 ||
                        neighbourX >= gridColumns) {
                        continue;
                    }

                    int neighbourIndex = neighbourY * gridColumns + neighbourX;
                    const std::vector<size_t> &neighborCell = grid[neighbourIndex];

                    for (size_t aIndex : currentCell) {
                        for (size_t bIndex : neighborCell) {
                            if (bIndex <= aIndex) {
                                continue;
                            }

                            Particle &a = particles[aIndex];
                            Particle &b = particles[bIndex];

                            Vec2 delta = b.position - a.position;
                            float distance = delta.length();
                            narrowphaseChecks++;

                            if (distance <= 0.0f || distance >= config.smoothingRadius) {
                                continue;
                            }

                            actualCollisions++;

                            float t = 1.0f - (distance / config.smoothingRadius);
                            a.density += t;
                            b.density += t;

                        }
                    }
                }
            }
        }
    }
}

void ParticleSystem::update(float dt) {
    narrowphaseChecks = 0;
    actualCollisions = 0;

    integrate(dt);
    buildGrid();
    computeDensities();
    if (config.mode == SimulationMode::Balls) {
        resolveBallCollisions();
    } else {
        applyPressureRepulsion(dt);
    }
    resolveWallCollisions();
}

void ParticleSystem::draw() const {
    if (config.mode == SimulationMode::Pressure) {
        return drawPressure();
    }
    constexpr float maxVisualSpeed = 600.0f;

    for (const Particle &particle : particles) {
        float speed = particle.velocity.length();
        float normalizedSpeed = speed / maxVisualSpeed;

        if (normalizedSpeed > 1.0f) {
            normalizedSpeed = 1.0f;
        }

        unsigned char red = static_cast<unsigned char>(255 * normalizedSpeed);
        unsigned char blue = static_cast<unsigned char>(255 * (1.0f - normalizedSpeed));

        Color color{red, 50, blue, 255};

        DrawCircleV({particle.position.x, particle.position.y}, particle.radius, color);
    }
}

void ParticleSystem::drawPressure() const {
    for (const Particle &particle : particles) {
        float normalizedDensity = particle.density / 10.0f;

        if (normalizedDensity > 1.0f) {
            normalizedDensity = 1.0f;
        }

        unsigned char red = static_cast<unsigned char>(255 * normalizedDensity);
        unsigned char blue = static_cast<unsigned char>(255 * (1.0f - normalizedDensity));

        Color color{red, 50, blue, 255};

        DrawCircleV({particle.position.x, particle.position.y}, particle.radius, color);
    }
}

void ParticleSystem::spawn(Vec2 position) {
    Particle p{
        .position = position,
        .velocity =
            {
                randomFloat(-250.0f, 250.0f),
                randomFloat(-250.0f, 50.0f),
            },
        .radius = randomFloat(3.0f, 8.0f),
    };

    particles.push_back(p);
}

int ParticleSystem::particleCount() const { return static_cast<int>(particles.size()); }

float ParticleSystem::randomFloat(float min, float max) {
    float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return min + t * (max - min);
}

int ParticleSystem::getNarrowphaseChecks() const { return narrowphaseChecks; }

int ParticleSystem::getActualCollisions() const { return actualCollisions; }

void ParticleSystem::drawGrid() const {
    for (int x = 0; x <= config.screenWidth; x += static_cast<int>(cellSize)) {
        DrawLine(x, 0, x, config.screenHeight, DARKGRAY);
    }
    for (int y = 0; y <= config.screenHeight; y += static_cast<int>(cellSize)) {
        DrawLine(0, y, config.screenWidth, y, DARKGRAY);
    }
}

void ParticleSystem::reset() {
    particles.clear();
    particles.reserve(config.particleCount);

    cellSize = config.gridSize;
    gridColumns = static_cast<int>(std::ceil(static_cast<float>(config.screenWidth) / cellSize));
    gridRows = static_cast<int>(std::ceil(static_cast<float>(config.screenHeight) / cellSize));
    grid.clear();
    grid.resize(gridColumns * gridRows);

    for (int i = 0; i < config.particleCount; i++) {
        Particle p{
            .position =
                {
                    randomFloat(0.0f, static_cast<float>(config.screenWidth)),
                    randomFloat(0.0f, static_cast<float>(config.screenHeight)),
                },
            .velocity =
                {
                    randomFloat(-200.0f, 200.0f),
                    randomFloat(-200.0f, 200.0f),
                },
            .radius = randomFloat(3.0f, 8.0f),
        };

        particles.push_back(p);
    }
}
