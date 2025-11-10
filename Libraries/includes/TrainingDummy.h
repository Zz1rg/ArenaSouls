#ifndef TRAINING_DUMMY_H
#define TRAINING_DUMMY_H

#include <glad/glad.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/shader_m.h>

#include <glm/glm.hpp>
#include "Player.h" // Include Player header for accessing player state

enum DummyState
{
    DUMMY_IDLE,
    DUMMY_IDLE_TO_GUARD,
    DUMMY_GUARDING,
    DUMMY_GUARD_TO_IDLE,
    DUMMY_GUARD_TO_PUNCH,
    DUMMY_PUNCHING,
    DUMMY_PUNCH_TO_GUARD,
    DUMMY_PUNCH_TO_IDLE,
    DUMMY_GOT_HIT // New state for handling got-hit animation
};

class TrainingDummy
{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float blendAmount;
    float blendRate;
    DummyState dummyState;

    Model model;
    Animation idleAnim;
    Animation guardAnim;
    Animation punchAnim;
    Animation gotHitAnim;

    Animator animator;

    // Distance thresholds
    float guardDistance; // Distance at which dummy puts up guard
    float punchDistance; // Distance at which dummy punches

    TrainingDummy();
    TrainingDummy(glm::vec3 startPosition);
    void update(float deltaTime, const Player& player);
    void draw(Shader& shader);
    float distanceToPlayer(glm::vec3 playerPosition);

    // Check if the training dummy is hit by the player
    bool isHitByPlayer(const Player& player);
};

#endif
