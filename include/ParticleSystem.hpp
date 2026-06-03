#pragma once

#include "Particle.hpp"
#include "SimulationConfig.hpp"
#include "Vec2.hpp"

#include <vector>

class ParticleSystem {
  public:
    explicit ParticleSystem(const SimulationConfig &config);

    void update(float dt);
    void draw() const;
    void spawn(Vec2 position);
    void reset();
    int particleCount() const;
    int getNarrowphaseChecks() const;
    int getActualCollisions() const;
    void drawGrid() const;
    void toggleMode();
    const char* modeName() const;
    SimulationConfig& getConfig();

  private:
    std::vector<Particle> particles;

    SimulationConfig config;

    static float randomFloat(float min, float max);
    void integrate(float dt);
    void resolveWallCollisions();
    void buildGrid();
    void resolveBallCollisions();
    void resolveParticlePair(Particle& a, Particle& b);
    void applyPressureRepulsion(float dt);
    void computeDensities();
    void drawPressure() const;

    float cellSize;
    int gridColumns;
    int gridRows;
    int narrowphaseChecks = 0;
    int actualCollisions = 0;

    std::vector<std::vector<size_t>> grid;
};
