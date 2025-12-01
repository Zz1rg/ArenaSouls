#include "TrainingDummy.h"
#include "Player.h"

#include <cmath>
#include <iostream>

TrainingDummy::TrainingDummy()
    : BaseEntity("resources/objects/dummy/idle/Breathing Idle.dae", &idleAnim),
    dummyState(DUMMY_IDLE),
    guardDistance(5.0f),
    punchDistance(2.0f),
    idleAnim("resources/objects/dummy/idle/Breathing Idle.dae", &model),
    guardAnim("resources/objects/dummy/guard/Bouncing Fight Idle.dae", &model),
    punchAnim("resources/objects/dummy/punch/Cross Punch.dae", &model),
    gotHitAnim("resources/objects/dummy/got-hit/Head Hit.dae", &model)
{
    position = glm::vec3(0.0f, -0.6f, -10.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = glm::vec3(0.6f);

    // Define hitboxes
    attackHitboxes.push_back(Hitbox({"mixamorig_Spine2",  glm::vec3(0.0f, 150.0f, 0.0f),0.5f}));
    blockHitboxes.push_back(Hitbox({"mixamorig_Spine2", glm::vec3(0.0f, 150.0f, 0.0f), 0.5f}));
}

float TrainingDummy::distanceToPlayer(glm::vec3 playerPosition) {
    return glm::length(playerPosition - position);
}

void TrainingDummy::update(float deltaTime) {
    // This is the new update function that will be called from main.cpp
    // It's empty for now, but you can add any logic that is not dependent on the player here.
}

void TrainingDummy::update(float deltaTime, const Player& player) {
    float distance = distanceToPlayer(player.position);
    isDamageActive = false;

    // Check if the training dummy is hit by the player
    if (isHitByPlayer(player) && dummyState != DUMMY_GOT_HIT && dummyState != DUMMY_GOT_HIT_TO_IDLE) {
        dummyState = DUMMY_GOT_HIT;
        animator.PlayAnimation(&gotHitAnim, NULL, 0.0f, 0.0f, 0.0f); // Reset and play animation
    }

    switch (dummyState) {
    case DUMMY_IDLE:
        animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
        if (distance <= guardDistance) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &guardAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            dummyState = DUMMY_IDLE_TO_GUARD;
        }
        break;

    case DUMMY_IDLE_TO_GUARD:
        handleAnimationBlend(&idleAnim, &guardAnim, dummyState, DUMMY_GUARDING);
        break;

    case DUMMY_GUARDING:
        animator.PlayAnimation(&guardAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
        if (distance <= punchDistance) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&guardAnim, &punchAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            dummyState = DUMMY_GUARD_TO_PUNCH;
        } else if (distance > guardDistance + 0.5f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&guardAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            dummyState = DUMMY_GUARD_TO_IDLE;
        }
        break;

    case DUMMY_GUARD_TO_IDLE:
        handleAnimationBlend(&guardAnim, &idleAnim, dummyState, DUMMY_IDLE);
        break;

    case DUMMY_GUARD_TO_PUNCH:
        handleAnimationBlend(&guardAnim, &punchAnim, dummyState, DUMMY_PUNCHING);
        break;

    case DUMMY_PUNCHING:
        {
            float animTime = animator.m_CurrentTime;
            float animDuration = punchAnim.GetDuration();
            // Activate damage window during the middle of the punch animation
            if (animTime >= animDuration * 0.4f && animTime <= animDuration * 0.6f) {
                isDamageActive = true;
            }
            animator.PlayAnimation(&punchAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
            if (animator.m_CurrentTime > punchAnim.GetDuration() - 0.1f) {
    
                blendAmount = 0.0f;
                if (distance <= guardDistance) {
                    animator.PlayAnimation(&punchAnim, &guardAnim, animator.m_CurrentTime, 0.0f, blendAmount);
                    dummyState = DUMMY_PUNCH_TO_GUARD;
                } else {
                    animator.PlayAnimation(&punchAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
                    dummyState = DUMMY_PUNCH_TO_IDLE;
                }
            }
        }
        break;

    case DUMMY_PUNCH_TO_GUARD:
        handleAnimationBlend(&punchAnim, &guardAnim, dummyState, DUMMY_GUARDING);
        break;

    case DUMMY_PUNCH_TO_IDLE:
        handleAnimationBlend(&punchAnim, &idleAnim, dummyState, DUMMY_IDLE);
        break;

    case DUMMY_GOT_HIT:
        animator.PlayAnimation(&gotHitAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
        if (animator.m_CurrentTime >= gotHitAnim.GetDuration() - 0.1f) {
            dummyState = DUMMY_GOT_HIT_TO_IDLE;
        }
        break;

    case DUMMY_GOT_HIT_TO_IDLE:
        if (handleAnimationBlend(&gotHitAnim, &idleAnim, dummyState, DUMMY_IDLE)) {
            // Blend completed
        }
        break;
    }

    animator.UpdateAnimation(deltaTime);

    // Update hitbox positions
    updateHitboxes(attackHitboxes);
    updateHitboxes(blockHitboxes);
}



bool TrainingDummy::isHitByPlayer(const Player& player) {
    if (!player.isDamageActive) {
        return false;
    }

    for (const auto& playerHitbox : player.attackHitboxes) {
        for (const auto& dummyHitbox : blockHitboxes) { // Using blockHitboxes as body hitboxes
            float distance = glm::length(playerHitbox.worldPosition - dummyHitbox.worldPosition);
            if (distance < (playerHitbox.radius + dummyHitbox.radius)) {
                return true;
            }
        }
    }

    return false;
}
