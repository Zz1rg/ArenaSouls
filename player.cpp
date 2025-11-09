#include "Player.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
//#include <learnopengl/filesystem.h>

Player::Player()
    : position(0.0f, -0.6f, 0.0f),
      blendAmount(0.0f),
      blendRate(0.055f),
      charState(IDLE),
      /*model(FileSystem::getPath("resources/objects/idle-mixamo/Warrior Idle.dae")),
      idleAnim(FileSystem::getPath("resources/objects/idle-mixamo/Warrior Idle.dae"), &model),
      walkAnim(FileSystem::getPath("resources/objects/walk-inplace-mixamo/Walking.dae"), &model),
      runAnim(FileSystem::getPath("resources/objects/run-inplace-mixamo/Fast Run.dae"), &model),
      rollAnim(FileSystem::getPath("resources/objects/roll-inplace-mixamo/Standing Dive Forward.dae"), &model),*/
      model("resources/objects/idle-mixamo/Warrior Idle.dae"),
      idleAnim("resources/objects/idle-mixamo/Warrior Idle.dae", &model),
      walkAnim("resources/objects/walk-inplace-mixamo/Walking.dae", &model),
      runAnim("resources/objects/run-inplace-mixamo/Fast Run.dae", &model),
      rollAnim("resources/objects/roll-inplace-mixamo/Standing Dive Forward.dae", &model),
      animator(&idleAnim)
{
}

void Player::processInput(GLFWwindow* window, float deltaTime)
{
    // You can add movement logic here if needed
}

void Player::update(float deltaTime)
{
    float walkSpeed = 2.0f;
    float rollSpeed = 5.0f;
    float runSpeed = 5.0f;

    switch (charState) {
    case IDLE:
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_E) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_WALK;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &rollAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_ROLL;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &runAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_RUN;
        }
        break;

    case IDLE_WALK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        position += glm::vec3(0.0f, 0.0f, -1.0f) * walkSpeed * deltaTime;
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&walkAnim, NULL, startTime, 0.0f, blendAmount);
            charState = WALK;
        }
        break;

    case WALK:
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_E) != GLFW_PRESS) {
            charState = WALK_IDLE;
        }
        break;

    case WALK_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&walkAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case IDLE_RUN:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &runAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&runAnim, NULL, startTime, 0.0f, blendAmount);
            charState = RUN;
        }
        break;

    case RUN:
        animator.PlayAnimation(&runAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) != GLFW_PRESS) {
            charState = RUN_IDLE;
        }
        break;

    case RUN_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&runAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case IDLE_ROLL:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &rollAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.7f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&rollAnim, NULL, startTime, 0.0f, blendAmount);
            charState = ROLL_IDLE;
        }
        break;

    case ROLL_IDLE:
        if (animator.m_CurrentTime > 0.7f) {
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&rollAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
            }
        }
        break;
    }

    animator.UpdateAnimation(deltaTime);
}

void Player::draw(Shader& shader)
{
    auto transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75f));
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}
