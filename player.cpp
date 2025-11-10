#include "Player.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>

Player::Player()
    : position(0.0f, -0.6f, 0.0f),
      blendAmount(0.0f),
      blendRate(0.055f),
      charState(IDLE),
      model("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae"), //
      idleAnim("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae", &model), //
      walkAnim("resources/objects/mixamo-knight/walk-inplace/Sword And Shield Walk.dae", &model), //
      runAnim("resources/objects/mixamo-knight/run/Sword And Shield Run.dae", &model), //
      rollAnim("resources/objects/roll-inplace-mixamo/Standing Dive Forward.dae", &model),
      attackAnim1("resources/objects/mixamo-knight/Attack1-fast/Sword And Shield Slash.dae", &model), //
      attackAnim2("resources/objects/mixamo-knight/Attack2-fast/Sword And Shield Slash.dae", &model), //
      attackAnim3("resources/objects/mixamo-knight/Attack3-fast/Sword And Shield Slash.dae", &model), //
      runAttackAnim("resources/objects/mixamo-knight/Attack4-fast/Sword And Shield Attack.dae", &model), //
	  initBlockAnim("resources/objects/mixamo-knight/init-block/Sword And Shield Block.dae", &model), //
	  blockAnim("resources/objects/mixamo-knight/block/Sword And Shield Block Idle.dae", &model), //
      animator(&idleAnim),
      chain(false)
{
}

void Player::processInput(GLFWwindow* window, float deltaTime)
{
	// movement logic for later implementation
}

void Player::tryBlock(Animation &transitAnim)
{
    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&transitAnim, &initBlockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockAnim, NULL, startTime, 0.0f, blendAmount);
            charState = BLOCK;
        }
    }
}

float chainWindowStart = 0.6f;

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
        /*else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &rollAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_ROLL;
        }*/
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &runAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_RUN;
        }
        // if left click attack
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &attackAnim1, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_ATTACK_1;
        }
        break;

    case IDLE_WALK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        //position += glm::vec3(0.0f, 0.0f, -1.0f) * walkSpeed * deltaTime;
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&walkAnim, NULL, startTime, 0.0f, blendAmount);
            charState = WALK;
        }
        break;

    case WALK:
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        position += glm::vec3(0.0f, 0.0f, 1.0f) * walkSpeed * deltaTime;
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_E) != GLFW_PRESS) {
            charState = WALK_IDLE;
        }
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            charState = WALK_ATTACK;
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

    case WALK_ATTACK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&walkAnim, &attackAnim1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim1, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_1;
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
		position += glm::vec3(0.0f, 0.0f, 1.0f) * runSpeed * deltaTime;
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) != GLFW_PRESS) {
            charState = RUN_IDLE;
        }
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            charState = RUN_ATTACK;
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

    case RUN_ATTACK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&runAnim, &runAttackAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim1, NULL, startTime, 0.0f, blendAmount);
            charState = RUN_ATTACKING;
        }
        break;

    case RUN_ATTACKING:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&runAttackAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > runAttackAnim.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;

            position += glm::vec3(0.0f, 0.0f, 1.0f) * 1.69f; // small forward movement on attack end

            if (chain) {
                animator.PlayAnimation(&runAttackAnim, &attackAnim3, startTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_3;
                chain = false;
            }
            else {
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
                blendAmount = 0.0f;
            }
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

    case IDLE_ATTACK_1:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &attackAnim1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim1, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_1;
        }
        break;

    case ATTACK_1:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim1.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;
            if (chain) {
                animator.PlayAnimation(&attackAnim1, &attackAnim2, startTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_2;
                chain = false;
            }
            else {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
            }
        }
        break;

    case CHAIN_ATTACK_2:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim1, &attackAnim2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            //blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim2, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_2;
        }
        break;

    case ATTACK_2:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim2, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim2.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;

            //position += glm::vec3(0.0f, 0.0f, 1.0f) * 1.69f; // small forward movement on attack end

            if (chain) {
                animator.PlayAnimation(&attackAnim1, &attackAnim2, startTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_3;
                chain = false;
            }
            else {
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
                blendAmount = 0.0f;
            }
        }
        break;

    case CHAIN_ATTACK_3:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim2, &attackAnim3, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim3, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_3;
            //blendAmount = 0.0f;
        }

    case ATTACK_3:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim3, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (animator.m_CurrentTime > attackAnim3.GetDuration() - 0.1f) {
            //blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            position += glm::vec3(0.0f, 0.0f, 1.0f) * 0.74f; // small forward movement on attack end
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_3_IDLE;
        }
        break;

    case ATTACK_3_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&attackAnim3, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            //blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
            blendAmount = 0.0f;
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

//glm::vec3 Player::getPosition()
//{
//    return position;
//}
