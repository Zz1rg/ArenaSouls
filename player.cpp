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
      dodgeAnim("resources/objects/mixamo-knight/dodge-back/Standing Dodge Backward.dae", &model),
      attackAnim1("resources/objects/mixamo-knight/Attack1-fast/Sword And Shield Slash.dae", &model), //
      attackAnim2("resources/objects/mixamo-knight/Attack2-fast/Sword And Shield Slash.dae", &model), //
      attackAnim3("resources/objects/mixamo-knight/Attack3-fast/Sword And Shield Slash.dae", &model), //
      runAttackAnim("resources/objects/mixamo-knight/Attack4-fast/Sword And Shield Attack.dae", &model), //
	  initBlockAnim("resources/objects/mixamo-knight/init-block/Sword And Shield Block.dae", &model), //
	  blockAnim("resources/objects/mixamo-knight/block/Sword And Shield Block Idle.dae", &model), //
	  blockWalkAnim("resources/objects/mixamo-knight/block-walk/Sword And Shield Strafe.dae", &model), //
      animator(&idleAnim),
      chain(false),
	  isBlocking(false)
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

void Player::tryBlock()
{
    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || isBlocking)
    {
        isBlocking = true;
        //charState = BLOCK;
        charState = INIT_BLOCK;
    }
}

void Player::tryDodge()
{
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        charState = START_DODGE;
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

    Animation* sourceAnim = getSourceAnimationForBlock(prevState);

    switch (charState) {
    case IDLE:
        currentAnim = &idleAnim;
        prevState = IDLE;
		tryBlock();
        tryDodge();
        animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, &walkAnim, startTime, 0.0f, blendAmount);
            charState = IDLE_WALK;
        }
        // if left click attack
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, &attackAnim1, startTime, 0.0f, blendAmount);
            charState = IDLE_ATTACK_1;
        }
        break;

    case IDLE_WALK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&walkAnim, NULL, startTime, 0.0f, blendAmount);
            charState = WALK;
        }
        break;

    case WALK:
        currentAnim = &walkAnim;
        prevState = WALK;
		tryBlock();
        tryDodge();
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (!isMoving()) {
            blendAmount = 0.0f;
            charState = WALK_IDLE;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            charState = WALK_RUN;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            charState = WALK_ATTACK;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            isBlocking = true;
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
        currentAnim = &runAnim;
        prevState = RUN;
		tryBlock();
        tryDodge();
        animator.PlayAnimation(&runAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (!isMoving()) {
            blendAmount = 0.0f;
            charState = RUN_IDLE;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            blendAmount = 0.0f;
            charState = RUN_WALK;
		}
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
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

    case START_DODGE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        //animator.PlayAnimation(getCurrentAnimation(), &dodgeAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        animator.PlayAnimation(currentAnim, &dodgeAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.7f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&dodgeAnim, NULL, startTime, 0.0f, blendAmount);
            charState = DODGE_END;
        }
        break;

    case DODGE_END:
        if (animator.m_CurrentTime > 0.7f) {
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&dodgeAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
                position += getForwardDir() * -0.74f;
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
        currentAnim = &attackAnim1;
        prevState = ATTACK_1;
		tryBlock();
        tryDodge(); 
        animator.PlayAnimation(&attackAnim1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim1.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;
            blendAmount = 0.0f;
            if (chain) {
                animator.PlayAnimation(&attackAnim1, &attackAnim2, startTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_2;
                chain = false;
            }
            else {
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
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim2, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_2;
        }
        break;

    case ATTACK_2:
        currentAnim = &attackAnim2;
        prevState = ATTACK_2;
        tryBlock();
        tryDodge();

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
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim3, NULL, startTime, 0.0f, blendAmount);
            charState = ATTACK_3;
        }
        break;

    case ATTACK_3:
        animator.PlayAnimation(&attackAnim3, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (animator.m_CurrentTime > attackAnim3.GetDuration() - 0.1f) {
            blendAmount = 0.0f;
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
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case INIT_BLOCK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        //animator.PlayAnimation(getCurrentAnimation(), &initBlockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        //animator.PlayAnimation(currentAnim, &initBlockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

        animator.PlayAnimation(currentAnim, &initBlockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&initBlockAnim, &blockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            //charState = INIT_BLOCKING;
            charState = INIT_BLOCK_TO_BLOCK;
        }
        break;

    //case INIT_BLOCKING:
    //    blendAmount += blendRate;
    //    blendAmount = fmod(blendAmount, 1.0f);
    //    animator.PlayAnimation(&initBlockAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
    //    if (animator.m_CurrentTime > initBlockAnim.GetDuration() - 0.1f) {
    //        blendAmount = 0.0f;
    //        float startTime = animator.m_CurrentTime2;
    //        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
    //            isBlocking = false;
    //            animator.PlayAnimation(&initBlockAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
    //            charState = INIT_BLOCK_IDLE;
    //        } else {
    //            animator.PlayAnimation(&initBlockAnim, &blockAnim, animator.m_CurrentTime, 0.0f, blendAmount);
    //            charState = INIT_BLOCK_TO_BLOCK;
    //            //charState = BLOCK;
    //        }
    //    }
    //    break;

    case INIT_BLOCK_TO_BLOCK:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(&initBlockAnim, &blockAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockAnim, NULL, startTime, 0.0f, blendAmount);
            charState = BLOCK;
        }
        break;

    case INIT_BLOCK_IDLE:
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        isBlocking = false;
        animator.PlayAnimation(&blockAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;


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
        isBlocking = false;
        animator.PlayAnimation(&blockAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount > 0.9f) {
            blendAmount = 0.0f;
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

Animation* Player::getSourceAnimationForBlock(AnimState prevState) {
    switch (prevState) {
        case IDLE: return &idleAnim;
        case WALK: return &walkAnim;
        case RUN: return &runAnim;
        case ATTACK_1: return &attackAnim1;
        case ATTACK_2: return &attackAnim2;
        case ATTACK_3: return &attackAnim3;
        // Add other cases as needed
        default: return &idleAnim; // Safe fallback
    }
}
