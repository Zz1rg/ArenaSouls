#include "BloodParticle.h"
#include <cmath>
#include <algorithm>
#include <random>

BloodParticleSystem::BloodParticleSystem() 
    : gen(std::random_device{}()), 
      angleDist(0.0f, 2.0f * 3.14159f),
      speedDist(2.0f, 8.0f),
      lifetimeDist(1.0f, 3.0f) {
}

BloodParticleSystem::BloodParticleSystem(const BloodParticleSystem& other) 
    : particles(other.particles),
      gen(std::random_device{}()),
      angleDist(other.angleDist),
      speedDist(other.speedDist),
      lifetimeDist(other.lifetimeDist) {
}

BloodParticleSystem& BloodParticleSystem::operator=(const BloodParticleSystem& other) {
    if (this != &other) {
        particles = other.particles;
        gen.seed(std::random_device{}());
        angleDist = other.angleDist;
        speedDist = other.speedDist;
        lifetimeDist = other.lifetimeDist;
    }
    return *this;
}

void BloodParticleSystem::spawnBloodSplatter(const glm::vec3& position, int particleCount) {
    for (int i = 0; i < particleCount; ++i) {
        // Random horizontal angle
        float angle = angleDist(gen);
        
        // Random speed
        float speed = speedDist(gen);
        
        // Create velocity with some upward component and random horizontal direction
        glm::vec3 velocity(
            std::cos(angle) * speed,
            std::abs(speedDist(gen)) * 0.5f + 1.0f, // Upward velocity (1-5 units)
            std::sin(angle) * speed
        );
        
        // Random lifetime
        float lifetime = lifetimeDist(gen);
        
        // Blood color (dark red)
        glm::vec3 color(0.8f, 0.1f, 0.1f);
        
        // Random size - smaller particles
        float size = 0.02f + (static_cast<float>(rand()) / RAND_MAX) * 0.04f; // 0.02 to 0.06
        
        // Add slight position variation
        glm::vec3 spawnPos = position + glm::vec3(
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f
        );
        
        particles.emplace_back(spawnPos, velocity, lifetime, color, size);
    }
}

void BloodParticleSystem::spawnBlockParticles(const glm::vec3& position, int particleCount) {
    for (int i = 0; i < particleCount; ++i) {
        // Random horizontal angle
        float angle = angleDist(gen);
        
        // Smaller, slower speeds for block particles
        float speed = 1.0f + (static_cast<float>(rand()) / RAND_MAX) * 2.0f; // 1-3 units speed
        
        // Create velocity with minimal upward component and smaller horizontal spread
        glm::vec3 velocity(
            std::cos(angle) * speed * 0.7f, // Reduced horizontal spread
            std::abs(static_cast<float>(rand()) / RAND_MAX) * 1.0f + 0.5f, // Small upward velocity (0.5-1.5 units)
            std::sin(angle) * speed * 0.7f  // Reduced horizontal spread
        );
        
        // Shorter lifetime for block particles
        float lifetime = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f; // 0.5-1.5 seconds
        
        // Gray color for regular block particles with slight variation
        float grayVariation = 0.6f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f; // 0.6 to 0.9 intensity
        glm::vec3 color(grayVariation, grayVariation, grayVariation); // Gray sparks
        
        // Smaller size for block particles
        float size = 0.01f + (static_cast<float>(rand()) / RAND_MAX) * 0.02f; // 0.01 to 0.03
        
        // Smaller position variation
        glm::vec3 spawnPos = position + glm::vec3(
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f
        );
        
        particles.emplace_back(spawnPos, velocity, lifetime, color, size);
    }
}

void BloodParticleSystem::spawnParryParticles(const glm::vec3& position, int particleCount) {
    for (int i = 0; i < particleCount; ++i) {
        // Random horizontal angle
        float angle = angleDist(gen);
        
        // Slightly faster and more spread out for parry particles
        float speed = 2.0f + (static_cast<float>(rand()) / RAND_MAX) * 3.0f; // 2-5 units speed
        
        // Create velocity with more upward component for dramatic effect
        glm::vec3 velocity(
            std::cos(angle) * speed,
            std::abs(static_cast<float>(rand()) / RAND_MAX) * 2.0f + 1.0f, // Higher upward velocity (1-3 units)
            std::sin(angle) * speed
        );
        
        // Longer lifetime for parry particles (more impressive effect)
        float lifetime = 1.0f + (static_cast<float>(rand()) / RAND_MAX) * 1.5f; // 1.0-2.5 seconds
        
        // Bright gold color for parry particles with slight variation
        float goldVariation = 0.9f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f; // 0.9 to 1.0 intensity
        glm::vec3 color(1.0f * goldVariation, 0.9f * goldVariation, 0.1f * goldVariation); // Bright gold
        
        // Slightly larger size for parry particles
        float size = 0.015f + (static_cast<float>(rand()) / RAND_MAX) * 0.025f; // 0.015 to 0.04
        
        // Larger position variation for more dramatic spread
        glm::vec3 spawnPos = position + glm::vec3(
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.4f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.4f,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.4f
        );
        
        particles.emplace_back(spawnPos, velocity, lifetime, color, size);
    }
}

void BloodParticleSystem::update(float deltaTime) {
    // Update all particles
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }
    
    // Remove dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const BloodParticle& p) { return !p.isAlive(); }),
        particles.end()
    );
}

void BloodParticleSystem::render(DebugDrawer& debugDrawer, const glm::mat4& view, const glm::mat4& projection) {
    for (const auto& particle : particles) {
        if (particle.isAlive()) {
            debugDrawer.drawSphere(particle.position, particle.size, particle.color, view, projection);
        }
    }
}

void BloodParticleSystem::clear() {
    particles.clear();
}