#include "TrainingDummy.h"

#include <cmath>
#include <iostream>

TrainingDummy::TrainingDummy()
    : position(-5.0f, -0.6f, 0.0f),
      blendAmount(0.0f),
      blendRate(0.055f),
      dummyState(DUMMY_IDLE),
      guardDistance(5.0f),  // Dummy guards when player is within 5 units
      punchDistance(2.0f),  // Dummy punches when player is within 2 units
      model("resources/objects/dummy/Breathing Idle.dae"),
      idleAnim("resources/objects/dummy/Breathing Idle.dae", &model),
      // TODO: Initialize these when you have the animation files:
      // guardAnim("resources/objects/dummy/guard.dae", &model),
      // punchAnim("resources/objects/dummy/punch.dae", &model),
      animator(&idleAnim) {}

TrainingDummy::TrainingDummy(glm::vec3 startPosition)
    : position(startPosition),
      blendAmount(0.0f),
      blendRate(0.055f),
      dummyState(DUMMY_IDLE),
      guardDistance(5.0f),
      punchDistance(2.0f),
      model("resources/objects/dummy/Breathing Idle.dae"),
      idleAnim("resources/objects/dummy/Breathing Idle.dae", &model),
      animator(&idleAnim) {}

float TrainingDummy::distanceToPlayer(glm::vec3 playerPosition) {
    return glm::length(playerPosition - position);
}

void TrainingDummy::update(float deltaTime, glm::vec3 playerPosition) {
    float distance = distanceToPlayer(playerPosition);

    switch (dummyState) {
        case DUMMY_IDLE:
            // Play idle animation
            animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime,
                                   0.0f, 0.0f);

            // Check if player is close enough to trigger guard
            if (distance <= guardDistance) {
                // TODO: When guard animation is available, transition to
                // DUMMY_IDLE_TO_GUARD For now, we'll just log it
                std::cout << "Player entered guard range! Distance: "
                          << distance << std::endl;
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
                std::cout << "Player entered punch range! Distance: "
                          << distance << std::endl;
                // Uncomment when punch animation is ready:
                // blendAmount = 0.0f;
                // animator.PlayAnimation(&guardAnim, &punchAnim,
                // animator.m_CurrentTime, 0.0f, blendAmount); dummyState =
                // DUMMY_GUARD_TO_PUNCH;
            }
            // Check if player moved away
            else if (distance >
                     guardDistance +
                         0.5f) {  // Small hysteresis to prevent flickering
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
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f,
                                       blendAmount);
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
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f,
                                       blendAmount);
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
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                       transforms[i]);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75f));
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}
