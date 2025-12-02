#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <random>
#include "DebugDrawer.h"

struct BloodParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    float maxLifetime;
    glm::vec3 color;
    float size;

    BloodParticle(const glm::vec3& pos, const glm::vec3& vel, float life, const glm::vec3& col, float sz)
        : position(pos), velocity(vel), lifetime(life), maxLifetime(life), color(col), size(sz) {}
    
    bool isAlive() const {
        return lifetime > 0.0f;
    }
    
    void update(float deltaTime) {
        // Apply gravity
        velocity.y -= 9.8f * deltaTime;
        
        // Update position
        position += velocity * deltaTime;
        
        // Decrease lifetime
        lifetime -= deltaTime;
        
        // Fade color over time
        float alpha = lifetime / maxLifetime;
        color = glm::vec3(0.8f, 0.1f, 0.1f) * alpha;  // Dark red that fades
    }
};

class BloodParticleSystem {
private:
    std::vector<BloodParticle> particles;
    mutable std::mt19937 gen;
    std::uniform_real_distribution<float> angleDist;
    std::uniform_real_distribution<float> speedDist;
    std::uniform_real_distribution<float> lifetimeDist;

public:
    BloodParticleSystem();
    BloodParticleSystem(const BloodParticleSystem& other);
    BloodParticleSystem& operator=(const BloodParticleSystem& other);
    
    void spawnBloodSplatter(const glm::vec3& position, int particleCount = 15);
    void update(float deltaTime);
    void render(DebugDrawer& debugDrawer, const glm::mat4& view, const glm::mat4& projection);
    void clear();
    
    size_t getParticleCount() const { return particles.size(); }
};