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
    model("resources/objects/dummy/Breathing Idle.dae"),
    idleAnim("resources/objects/dummy/Breathing Idle.dae", &model),
    // TODO: Initialize these when you have the animation files:
    // guardAnim("resources/objects/dummy/guard.dae", &model),
    // punchAnim("resources/objects/dummy/punch.dae", &model),
    animator(&idleAnim) {}

float TrainingDummy::distanceToPlayer(glm::vec3 playerPosition) {
    return glm::length(playerPosition - position);
}

void TrainingDummy::update(float deltaTime, const Player& player) {
    float distance = distanceToPlayer(player.position);

    // Check if the training dummy is hit by the player
    if (isHitByPlayer(player)) {
        std::cout << "Training Dummy was hit during update!" << std::endl;
    }

    switch (dummyState) {
    case DUMMY_IDLE:
        // Play idle animation
        animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);

        // Check if player is close enough to trigger guard
        if (distance <= guardDistance) {
            // TODO: When guard animation is available, transition to
            // DUMMY_IDLE_TO_GUARD For now, we'll just log it
            std::cout << "Player entered guard range! Distance: " << distance << std::endl;
            // Uncomment when guard animation is ready:
            // blendAmount = 0.0f;
            // animator.PlayAnimation(&idleAnim, &guardAnim,
            // animator.m_CurrentTime, 0.0f, blendAmount); dummyState =
            // DUMMY_IDLE_TO_GUARD;
        }
        break;

    case DUMMY_IDLE_TO_GUARD:
        // TODO: Implement when guard animation is available
        // Blend from idle to guard animation
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        // animator.PlayAnimation(&idleAnim, &guardAnim,
        // animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            // animator.PlayAnimation(&guardAnim, NULL, startTime, 0.0f,
            // blendAmount);
            dummyState = DUMMY_GUARDING;
        }
        break;

    case DUMMY_GUARDING:
        // TODO: Play guard animation when available
        // animator.PlayAnimation(&guardAnim, NULL, animator.m_CurrentTime,
        // 0.0f, 0.0f);

        // Check if player is close enough to punch
        if (distance <= punchDistance) {
            std::cout << "Player entered punch range! Distance: " << distance << std::endl;
            // Uncomment when punch animation is ready:
            // blendAmount = 0.0f;
            // animator.PlayAnimation(&guardAnim, &punchAnim,
            // animator.m_CurrentTime, 0.0f, blendAmount); dummyState =
            // DUMMY_GUARD_TO_PUNCH;
        }
        // Check if player moved away
        else if (distance > guardDistance + 0.5f) { // Small hysteresis to prevent flickering
            blendAmount = 0.0f;
            // animator.PlayAnimation(&guardAnim, &idleAnim,
            // animator.m_CurrentTime, 0.0f, blendAmount);
            dummyState = DUMMY_GUARD_TO_IDLE;
        }
        break;

    case DUMMY_GUARD_TO_IDLE:
        // TODO: Implement when guard animation is available
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        // animator.PlayAnimation(&guardAnim, &idleAnim,
        // animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            dummyState = DUMMY_IDLE;
        }
        break;

    case DUMMY_GUARD_TO_PUNCH:
        // TODO: Implement when punch animation is available
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        // animator.PlayAnimation(&guardAnim, &punchAnim,
        // animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            // animator.PlayAnimation(&punchAnim, NULL, startTime, 0.0f,
            // blendAmount);
            dummyState = DUMMY_PUNCHING;
        }
        break;

    case DUMMY_PUNCHING:
        // TODO: Play punch animation when available
        // animator.PlayAnimation(&punchAnim, NULL, animator.m_CurrentTime,
        // 0.0f, 0.0f);

        // Check if punch animation is finished (you'll need to get the
        // animation duration) For now, we'll use a simple time-based
        // approach if (animator.m_CurrentTime > punchAnim.GetDuration() -
        // 0.1f) {
        //     blendAmount = 0.0f;
        //
        //     // Decide next state based on player distance
        //     if (distance <= guardDistance) {
        //         animator.PlayAnimation(&punchAnim, &guardAnim,
        //         animator.m_CurrentTime, 0.0f, blendAmount); dummyState =
        //         DUMMY_PUNCH_TO_GUARD;
        //     } else {
        //         animator.PlayAnimation(&punchAnim, &idleAnim,
        //         animator.m_CurrentTime, 0.0f, blendAmount); dummyState =
        //         DUMMY_PUNCH_TO_IDLE;
        //     }
        // }
        break;

    case DUMMY_PUNCH_TO_GUARD:
        // TODO: Implement when animations are available
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        // animator.PlayAnimation(&punchAnim, &guardAnim,
        // animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            // animator.PlayAnimation(&guardAnim, NULL, startTime, 0.0f,
            // blendAmount);
            dummyState = DUMMY_GUARDING;
        }
        break;

    case DUMMY_PUNCH_TO_IDLE:
        // TODO: Implement when punch animation is available
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        // animator.PlayAnimation(&punchAnim, &idleAnim,
        // animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            dummyState = DUMMY_IDLE;
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

    if (isHit) {
        std::cout << "Training Dummy got hit by the player!" << std::endl;
    }

    return isHit;
}
