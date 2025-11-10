#ifndef TRAINING_DUMMY_H
#define TRAINING_DUMMY_H

#include <glad/glad.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/shader_m.h>

#include <glm/glm.hpp>

enum DummyState
{
    DUMMY_IDLE,
    DUMMY_IDLE_TO_GUARD,
    DUMMY_GUARDING,
    DUMMY_GUARD_TO_IDLE,
    DUMMY_GUARD_TO_PUNCH,
    DUMMY_PUNCHING,
    DUMMY_PUNCH_TO_GUARD,
    DUMMY_PUNCH_TO_IDLE
};

class TrainingDummy
{
public:
    glm::vec3 position;
    glm::vec3 rotation; // Added rotation property
    float blendAmount;
    float blendRate;
    DummyState dummyState;

    Model model;
    Animation idleAnim;
    // TODO: Add these animations when you have the files:
    // Animation guardAnim;
    // Animation punchAnim;

    Animator animator;

    // Distance thresholds
    float guardDistance; // Distance at which dummy puts up guard
    float punchDistance; // Distance at which dummy punches

    TrainingDummy();
    TrainingDummy(glm::vec3 startPosition);
    void update(float deltaTime, glm::vec3 playerPosition);
    void draw(Shader& shader);
    float distanceToPlayer(glm::vec3 playerPosition);
};

#endif
