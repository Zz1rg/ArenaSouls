#include "Player.h"
#include "SoundEngine.h"
#include <stb_image.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player()
    : health(100),
      stamina(100.0f),
      maxHealth(100),
      maxStamina(100.0f),
      staminaRegenRate(25.0f),
      staminaRegenDelay(2.0f),
      lastStaminaUse(0.0f),
      position(0.0f, -0.6f, 0.0f),
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
      parryAnim("resources/objects/mixamo-knight/parry/Sword And Shield Impact.dae", &model), //
      isHitAnim("resources/objects/mixamo-knight/is-hit/Sword And Shield Impact.dae", &model), //
      animator(&idleAnim),
      chain(false),
	  isBlocking(false),
      soundEngine(nullptr)
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

    // --- Determine speed based on state and consume stamina ---
    float speed = 0.0f;
    switch (charState)
    {
    case WALK: case WALK_IDLE: case WALK_ATTACK: case WALK_RUN: case IDLE_WALK:
        speed = 2.0f;
        // Play footstep sound for walking
        if (glm::length(moveDir) > 0.0f && soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }
        break;
    case RUN: case RUN_IDLE: case RUN_WALK: case RUN_ATTACK: case IDLE_RUN:
        // Force transition to walk if stamina is depleted
        if (stamina <= 0.0f && charState == RUN) {
            speed = 2.0f;
            charState = RUN_WALK; // Transition back to walk
        } else {
            speed = 3.5f;
            // Only consume stamina when actually moving
            if (glm::length(moveDir) > 0.0f) {
                stamina -= 20.0f * deltaTime;
                if (stamina < 0.0f) stamina = 0.0f;
                lastStaminaUse = glfwGetTime();
                
                // Play footstep sound for running (faster pace than walking)
                if (soundEngine && !soundEngine->isSoundPlaying("footstep_fast")) {
                    soundEngine->playSound("footstep_fast");
                }
            }
        }
        break;
    case BLOCK_WALK: case BLOCK_WALKING:
        speed = 1.75f;
        // Consume stamina while blocking
        stamina -= 5.0f * deltaTime; // Light stamina drain for blocking
        if (stamina < 0.0f) {
            stamina = 0.0f;
            isBlocking = false; // Stop blocking when out of stamina
        }
        lastStaminaUse = glfwGetTime();
        break;
    default:
        speed = 0.0f;
        break;
    }
    
    // Update stamina regeneration
    updateStamina(deltaTime);

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
    if ((glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || isBlocking) && stamina > 0.0f)
    {
        // Stop movement sounds when blocking
        if (!isBlocking && soundEngine) {
            soundEngine->stopSound("footstep");
            soundEngine->stopSound("footstep_fast");
        }
        // Play block sound when starting to block
        // if (!isBlocking && soundEngine) {
        //     soundEngine->playSound("block");
        // }
        isBlocking = true;
        //charState = BLOCK;
        charState = INIT_BLOCK;
    }
}

void Player::tryDodge()
{
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS && stamina > 0.0f)
    {
        // Stop movement sounds when dodging
        if (soundEngine) {
            soundEngine->stopSound("footstep");
            soundEngine->stopSound("footstep_fast");
            soundEngine->playSound("dodge");
        }
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

void Player::updateStamina(float deltaTime) {
    // Regenerate stamina if we haven't used it recently
    float currentTime = glfwGetTime();
    if (currentTime - lastStaminaUse > staminaRegenDelay && stamina < maxStamina) {
        stamina += staminaRegenRate * deltaTime;
        if (stamina > maxStamina) stamina = maxStamina;
    }
}

void Player::setSoundEngine(SoundEngine* engine) {
    soundEngine = engine;
}

float chainWindowStart = 0.6f;

void Player::update(float deltaTime)
{

    float attackStaminaCost = 10.0f;
    float runAttackStaminaCost = 12.5f;
    float dodgeStaminaCost = 10.0f;

    switch (charState) {
    case IDLE:
        currentAnim = &idleAnim;
        // Stop any movement sounds that might be playing
        if (soundEngine) {
            soundEngine->stopSound("footstep");
            soundEngine->stopSound("footstep_fast");
        }
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
        // if left click attack (only if stamina > 0)
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, &attackAnim1, startTime, 0.0f, blendAmount);
            charState = IDLE_ATTACK_1;
        }
        // Test keys for health/stamina (for debugging)
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_H) == GLFW_PRESS) {
            health -= 10; // Decrease health for testing
            if (health < 0) health = 0;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_J) == GLFW_PRESS) {
            health += 10; // Increase health for testing
            if (health > maxHealth) health = maxHealth;
        }

        // Test keys for parry and hit reactions
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_P) == GLFW_PRESS) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, &parryAnim, startTime, 0.0f, blendAmount);
            charState = PARRY;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_I) == GLFW_PRESS) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, &isHitAnim, startTime, 0.0f, blendAmount);
            charState = IS_HIT;
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
		tryBlock();
        tryDodge();
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (!isMoving()) {
            blendAmount = 0.0f;
            charState = WALK_IDLE;
            if (soundEngine) {
                soundEngine->stopSound("footstep");
            }
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && stamina > 0.0f) {
            blendAmount = 0.0f;
            charState = WALK_RUN;
            if (soundEngine) {
                soundEngine->stopSound("footstep");
            }
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
            blendAmount = 0.0f;
            charState = WALK_ATTACK;
            if (soundEngine) {
                soundEngine->stopSound("footstep");
            }
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && stamina > 0.0f) {
            blendAmount = 0.0f;
            isBlocking = true;
            charState = BLOCK_WALK;
            if (soundEngine) {
                soundEngine->stopSound("footstep");
            }
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
            // Ensure all movement sounds are stopped when going to IDLE
            if (soundEngine) {
                soundEngine->stopSound("footstep");
                soundEngine->stopSound("footstep_fast");
            }
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
            // Play sword attack sound
            if (soundEngine) {
                soundEngine->playSound("sword_hit");
            }
            // Stop walking sounds when attacking
            if (soundEngine) {
                soundEngine->stopSound("footstep");
            }
            // Consume stamina
            stamina -= attackStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();
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
		tryBlock();
        tryDodge();
        animator.PlayAnimation(&runAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (!isMoving()) {
            blendAmount = 0.0f;
            charState = RUN_IDLE;
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            blendAmount = 0.0f;
            charState = RUN_WALK;
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
		}
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            blendAmount = 0.0f;
            charState = RUN_ATTACK;
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
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
            // Ensure all movement sounds are stopped when going to IDLE
            if (soundEngine) {
                soundEngine->stopSound("footstep");
                soundEngine->stopSound("footstep_fast");
            }
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
            // Stop fast footsteps when transitioning to walk
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
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
            // Play sword attack sound
            if (soundEngine) {
                soundEngine->playSound("sword_hit");
            }
            // Stop running sounds when attacking
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
            // Consume stamina
            stamina -= runAttackStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();
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
            // Consume stamina
            stamina -= dodgeStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();
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
            // Play sword attack sound
            if (soundEngine) {
                soundEngine->playSound("sword_hit");
            }
            charState = ATTACK_1;
        }
        break;

    case ATTACK_1:
        currentAnim = &attackAnim1;
		tryBlock();
        tryDodge(); 
        animator.PlayAnimation(&attackAnim1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim1.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;
            blendAmount = 0.0f;
            // Consume stamina 
            stamina -= attackStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();
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
            // Play sword attack sound
            if (soundEngine) {
                soundEngine->playSound("sword_hit");
            }
            charState = ATTACK_2;
        }
        break;

    case ATTACK_2:
        currentAnim = &attackAnim2;
        tryBlock();
        tryDodge();

        animator.PlayAnimation(&attackAnim2, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
            chain = true;
        }
        if (animator.m_CurrentTime > attackAnim2.GetDuration() - 0.1f) {
            float startTime = animator.m_CurrentTime2;

            // Consume stamina
            stamina -= attackStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();

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
            // Play sword attack sound
            if (soundEngine) {
                soundEngine->playSound("sword_hit");
            }
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
            // Consume stamina 
            stamina -= attackStaminaCost;
            if (stamina < 0.0f) stamina = 0.0f;
            lastStaminaUse = glfwGetTime();
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
        if (soundEngine) {
            soundEngine->stopSound("footstep");
        }
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
        if (soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }
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
        if (soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }

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

    case PARRY:
        animator.PlayAnimation(&parryAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        isBlocking = false;
        if (animator.m_CurrentTime > parryAnim.GetDuration() - 0.1f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case IS_HIT:
        animator.PlayAnimation(&isHitAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        isBlocking = false;
        if (animator.m_CurrentTime > isHitAnim.GetDuration() - 0.1f) {
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
