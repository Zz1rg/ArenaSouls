#include "Player.h"
#include <stb_image.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player()
    : position(0.0f, -0.6f, 0.0f),
	  rotation(0.0f, 180.0f, 0.0f),
      scale(0.6f),
      blendAmount(0.0f),
      blendRate(0.055f),
      charState(IDLE),
      model("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae"), //
      idleAnim("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae", &model), //
      walkAnim("resources/objects/mixamo-knight/walk-inplace/Sword And Shield Walk.dae", &model), //
      runAnim("resources/objects/mixamo-knight/run/Sword And Shield Run.dae", &model), //
      //rollAnim("resources/objects/roll-inplace-mixamo/Standing Dive Forward.dae", &model),
      attackAnim1("resources/objects/mixamo-knight/Attack1-fast/Sword And Shield Slash.dae", &model), //
      attackAnim2("resources/objects/mixamo-knight/Attack2-fast/Sword And Shield Slash.dae", &model), //
      attackAnim3("resources/objects/mixamo-knight/Attack3-fast/Sword And Shield Slash.dae", &model), //
      runAttackAnim("resources/objects/mixamo-knight/Attack4-fast/Sword And Shield Attack.dae", &model), //
	  initBlockAnim("resources/objects/mixamo-knight/init-block/Sword And Shield Block.dae", &model), //
	  blockAnim("resources/objects/mixamo-knight/block/Sword And Shield Block Idle.dae", &model), //
	  blockWalkAnim("resources/objects/mixamo-knight/block-walk/Sword And Shield Strafe.dae", &model), //
      animator(&idleAnim),
      chain(false),
	  isBlocking(false),
      currentSpeed(0.0f)
{
    /*stbi_set_flip_vertically_on_load(true);*/
}

void Player::processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    glm::vec3 moveDir(0.0f);

    // --- Get input ---
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDir.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir.x += 1.0f;

    if (glm::length(moveDir) > 0.0f)
        moveDir = glm::normalize(moveDir);

    // --- Determine speed based on state ---
    float speed = 0.0f;
    switch (charState)
    {
    case WALK: case WALK_IDLE: case WALK_ATTACK: case WALK_RUN: case IDLE_WALK:
        speed = 2.0f;
        break;
    case RUN: case RUN_IDLE: case RUN_WALK: case RUN_ATTACK: case IDLE_RUN:
        speed = 3.5f;
        break;
    case BLOCK_WALK: case BLOCK_WALKING:
        speed = 1.75f;
        break;
    default:
        speed = 0.0f;
        break;
    }

    // --- Camera-relative movement ---
    glm::vec3 camForward = camera.Front;
    camForward.y = 0.0f;
    camForward = glm::normalize(camForward);

    glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 worldMove = moveDir.x * camRight + moveDir.z * camForward;

    // --- Apply movement ---
    position += worldMove * speed * deltaTime;

    // --- Rotation logic ---
    float targetYaw = glm::degrees(atan2(camera.Front.x, camera.Front.z));
    rotation.y = targetYaw;
}

void Player::tryBlock(Animation &transitAnim)
{
    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || isBlocking)
    {
        isBlocking = true;
        //tryBlocking = true;
        blendAmount += 0.005f;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&transitAnim, &blockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
   //     if (blendAmount > 0.9f) {
			//blendAmount = 0.0f;
			//float startTime = animator.m_CurrentTime2;
			////animator.PlayAnimation(&initBlockAnim, NULL, startTime, 0.0f, blendAmount);
   //         charState = BLOCK;
   //     }
		charState = BLOCK;
    }
}

glm::vec3 Player::getForwardDir() {
    return glm::normalize(glm::vec3(
        sin(glm::radians(rotation.y)),
        0.0f,
        cos(glm::radians(rotation.y))
    ));
}

float chainWindowStart = 0.6f;

void Player::update(float deltaTime)
{
    float walkSpeed = 2.0f;
    float rollSpeed = 5.0f;
    float runSpeed = 5.0f;
	float blockSpeed = 1.25f;
	float runningAttackSpeed = 0.0f;
	float attack3Speed = 0.0f;

    switch (charState) {
    case IDLE:
		tryBlock(idleAnim);
        if (charState == BLOCK) { break; } // exit if blocking initiated
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_WALK;
        }
        /*else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            blendAmount = 0.0f;
            animator.PlayAnimation(&idleAnim, &rollAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE_ROLL;
        }*/
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
		tryBlock(walkAnim);
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        //position += glm::vec3(0.0f, 0.0f, 1.0f) * walkSpeed * deltaTime;
		currentSpeed = walkSpeed;
        if (!isMoving()) {
            charState = WALK_IDLE;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            charState = WALK_RUN;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            charState = WALK_ATTACK;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            isBlocking = true;
            animator.PlayAnimation(&walkAnim, &blockWalkAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = BLOCK_WALK;
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


    case WALK_RUN:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&walkAnim, &runAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&runAnim, NULL, startTime, 0.0f, blendAmount);
            charState = RUN;
        }
        break;

    case RUN:
		tryBlock(runAnim);
        animator.PlayAnimation(&runAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (!isMoving()) {
            charState = RUN_IDLE;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            charState = RUN_WALK;
		}
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
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

    case RUN_WALK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&runAnim, &walkAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&walkAnim, NULL, startTime, 0.0f, blendAmount);
            charState = WALK;
        }
        break;

    case RUN_ATTACK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&runAnim, &runAttackAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&runAttackAnim, NULL, startTime, 0.0f, blendAmount);
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

            // update character pos first
			position += getForwardDir() * 1.69f;

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

        /*case IDLE_ROLL:
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
            break;*/

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
		tryBlock(attackAnim1);
        animator.PlayAnimation(&attackAnim1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim1.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;
            if (chain) {
                blendAmount = 0.0f;
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
        tryBlock(attackAnim2);
        /*blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);*/
        animator.PlayAnimation(&attackAnim2, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim2.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;

            if (chain) {
                animator.PlayAnimation(&attackAnim2, &attackAnim3, startTime, 0.0f, blendAmount);
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
        /*blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);*/
        animator.PlayAnimation(&attackAnim3, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		position += getForwardDir() * attack3Speed * deltaTime;
        if (animator.m_CurrentTime > attackAnim3.GetDuration() - 0.1f) {
            //blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            position += getForwardDir() * 0.74f; // small forward movement on attack end
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

    //case INIT_BLOCK:
    //    blendAmount = 0.0f;
    //    animator.PlayAnimation(&initBlockAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
    //    if (animator.m_CurrentTime > initBlockAnim.GetDuration() - 0.1) {
    //        float startTime = animator.m_CurrentTime2;
    //        animator.PlayAnimation(&initBlockAnim, &blockAnim, startTime, 0.0f, blendAmount);
    //        charState = BLOCK;
    //    }
    //    break;

    //case INIT_BLOCK_TO_BLOCK:
    //    blendAmount += blendRate;
    //    blendAmount = fmod(blendAmount, 1.0f);
    //    animator.PlayAnimation(&idleAnim, &attackAnim1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
    //    if (blendAmount > 0.9f) {
    //        blendAmount = 0.0f;
    //        float startTime = animator.m_CurrentTime2;
    //        animator.PlayAnimation(&attackAnim1, NULL, startTime, 0.0f, blendAmount);
    //        charState = ATTACK_1;
    //    }
    //    break;


    case BLOCK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&blockAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
            blendAmount = 0.0f;
            isBlocking = false;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            charState = BLOCK_IDLE;
		}

        else if (isMoving()) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&blockAnim, &blockWalkAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = BLOCK_WALK;
		}
        break;

    case BLOCK_WALK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&blockAnim, &blockWalkAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockWalkAnim, NULL, startTime, 0.0f, blendAmount);
            charState = BLOCK_WALKING;
        }
        break;

    case BLOCK_WALKING:
        animator.PlayAnimation(&blockWalkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        if (!isMoving()) {
            charState = BLOCK;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
            blendAmount = 0.0f;
            isBlocking = false;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockWalkAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            charState = IDLE;
        }
        break;
        

    case BLOCK_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&blockAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            //blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
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
    modelMatrix = glm::scale(modelMatrix, scale);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}

//glm::vec3 Player::getPosition()
//{
//    return position;
//}

bool Player::isAttacking() const {
    return charState == ATTACK_1 || charState == ATTACK_2 || charState == ATTACK_3 ||
           charState == IDLE_ATTACK_1 || charState == CHAIN_ATTACK_2 || charState == CHAIN_ATTACK_3 ||
           charState == RUN_ATTACK || charState == RUN_ATTACKING || charState == WALK_ATTACK;
}

bool Player::isBlockingState() const {
    return charState == BLOCK || charState == BLOCK_WALK || charState == BLOCK_IDLE || isBlocking;
}

bool Player::isMoving() const {
    return glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS ||
           glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS;
}
