#include "TrainingDummy.h"
#include "Player.h" // Include Player header for accessing player state

#include <cmath>
#include <iostream>

TrainingDummy::TrainingDummy()
    : position(0.0f, -0.6f, -10.0f),
    rotation(0.0f, 0.0f, 0.0f),
    scale(0.6f),
    blendAmount(0.0f),
    blendRate(0.055f),
    dummyState(DUMMY_IDLE),
    guardDistance(5.0f),
    punchDistance(2.0f),
    model("resources/objects/dummy/idle/Breathing Idle.dae"),
    idleAnim("resources/objects/dummy/idle/Breathing Idle.dae", &model),
    guardAnim("resources/objects/dummy/guard/Bouncing Fight Idle.dae", &model),
    punchAnim("resources/objects/dummy/punch/Cross Punch.dae", &model),
    gotHitAnim("resources/objects/dummy/got-hit/Head Hit.dae", &model),
    animator(&idleAnim) {}

float TrainingDummy::distanceToPlayer(glm::vec3 playerPosition) {
    return glm::length(playerPosition - position);
}

void TrainingDummy::update(float deltaTime, const Player& player) {
    float distance = distanceToPlayer(player.position);

    // Check if the training dummy is hit by the player
    if (isHitByPlayer(player) || (dummyState == DUMMY_PUNCHING && player.isBlockingState())) {
        std::cout << "Training Dummy was hit during update!" << std::endl;
        // Play the got-hit animation
        animator.PlayAnimation(&gotHitAnim, NULL, 0.0f, 0.0f, 0.0f); // Reset animation time to start
        dummyState = DUMMY_GOT_HIT; // Transition to got-hit state
        return;
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
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &guardAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&guardAnim, NULL, animator.m_CurrentTime2, 0.0f, 0.0f);
            dummyState = DUMMY_GUARDING;
        }
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
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&guardAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime2, 0.0f, 0.0f);
            dummyState = DUMMY_IDLE;
        }
        break;

    case DUMMY_GUARD_TO_PUNCH:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&guardAnim, &punchAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&punchAnim, NULL, animator.m_CurrentTime2, 0.0f, 0.0f);
            dummyState = DUMMY_PUNCHING;
        }
        break;

    case DUMMY_PUNCHING:
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
        break;

    case DUMMY_PUNCH_TO_GUARD:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&punchAnim, &guardAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&guardAnim, NULL, animator.m_CurrentTime2, 0.0f, 0.0f);
            dummyState = DUMMY_GUARDING;
        }
        break;

    case DUMMY_PUNCH_TO_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&punchAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime2, 0.0f, 0.0f);
            dummyState = DUMMY_IDLE;
        }
        break;

    case DUMMY_GOT_HIT:
        blendAmount = 0.0f;
        animator.PlayAnimation(&gotHitAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
        if (animator.m_CurrentTime > gotHitAnim.GetDuration() - 0.1f) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&gotHitAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            dummyState = DUMMY_IDLE_TO_GUARD; // Transition back to idle-to-guard for smooth recovery
        }
        break;
    }

    // Update the animator
    animator.UpdateAnimation(deltaTime);
}

void TrainingDummy::draw(Shader& shader) {
    auto transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Added rotation
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}

bool TrainingDummy::isHitByPlayer(const Player& player) {
    // TODO: Update to use dynamic hitboxes based on player animations

    // Check if the player is in an attack state
    if (!player.isAttacking()) {
        return false;
    }

    // Check for collision between player and training dummy
    glm::vec3 playerPosition = player.position;
    glm::vec3 playerScale = player.scale;
    glm::vec3 dummyScale = scale;

    glm::vec3 playerMin = playerPosition - playerScale * 0.5f;
    glm::vec3 playerMax = playerPosition + playerScale * 0.5f;

    glm::vec3 dummyMin = position - dummyScale * 0.5f;
    glm::vec3 dummyMax = position + dummyScale * 0.5f;

    // Check for AABB (Axis-Aligned Bounding Box) collision
    bool collisionX = playerMax.x >= dummyMin.x && playerMin.x <= dummyMax.x;
    bool collisionY = playerMax.y >= dummyMin.y && playerMin.y <= dummyMax.y;
    bool collisionZ = playerMax.z >= dummyMin.z && playerMin.z <= dummyMax.z;

    bool isHit = collisionX && collisionY && collisionZ;


    return isHit;
}
