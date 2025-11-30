#ifndef TRAINING_DUMMY_H
#define TRAINING_DUMMY_H

#include "Entity.h"
#include "Hitbox.h"
#include <vector>

class Player; // Forward declaration

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
    DUMMY_GOT_HIT, // New state for handling got-hit animation
    DUMMY_GOT_HIT_TO_IDLE
};

class TrainingDummy : public BaseEntity
{
public:
    DummyState dummyState;

    Animation idleAnim;
    Animation guardAnim;
    Animation punchAnim;
    Animation gotHitAnim;

    std::vector<Hitbox> attackHitboxes;
    std::vector<Hitbox> blockHitboxes;

    // Distance thresholds
    float guardDistance; // Distance at which dummy puts up guard
    float punchDistance; // Distance at which dummy punches

    TrainingDummy();
    void update(float deltaTime, const Player& player);
    void update(float deltaTime) override;
    float distanceToPlayer(glm::vec3 playerPosition);

    // Check if the training dummy is hit by the player
    bool isHitByPlayer(const Player& player);
    bool isDamageActive = false;
};

#endif
