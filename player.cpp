#include "Player.h"
#include "Boss.h"
#include "SoundEngine.h"
#include <stb_image.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player()
    : BaseEntity("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae", &idleAnim), 
    health(100),
      stamina(100.0f),
      maxHealth(100),
      maxStamina(100.0f),
      staminaRegenRate(25.0f),
      staminaRegenDelay(2.0f),
      lastStaminaUse(0.0f),
      charState(IDLE),
      idleAnim("resources/objects/mixamo-knight/Sword And Shield Idle/Sword And Shield Idle.dae", &model),
      walkAnim("resources/objects/mixamo-knight/walk-inplace/Sword And Shield Walk.dae", &model),
      runAnim("resources/objects/mixamo-knight/run/Sword And Shield Run.dae", &model),
      dodgeAnim("resources/objects/mixamo-knight/dodge-back/Standing Dodge Backward.dae", &model),
      attackAnim1("resources/objects/mixamo-knight/Attack1-fast/Sword And Shield Slash.dae", &model),
      attackAnim2("resources/objects/mixamo-knight/Attack2-fast/Sword And Shield Slash.dae", &model),
      attackAnim3("resources/objects/mixamo-knight/Attack3-fast/Sword And Shield Slash.dae", &model),
      runAttackAnim("resources/objects/mixamo-knight/Attack4-fast/Sword And Shield Attack.dae", &model),
	  initBlockAnim("resources/objects/mixamo-knight/init-block/Sword And Shield Block.dae", &model),
	  blockAnim("resources/objects/mixamo-knight/block/Sword And Shield Block Idle.dae", &model),
	  blockWalkAnim("resources/objects/mixamo-knight/block-walk/Sword And Shield Strafe.dae", &model),
      parryAnim("resources/objects/mixamo-knight/parry/Sword And Shield Impact.dae", &model),
      isHitAnim("resources/objects/mixamo-knight/is-hit/Sword And Shield Impact.dae", &model),
      deadAnim("resources/objects/mixamo-knight/death/Falling Back Death.dae", &model),
      chain(false),
	  isBlocking(false),
      soundEngine(nullptr),
      hasHitTarget(false),
      isTakingHit(false),
      inParryWindow(false),
      parryWindowStart(0.0f),
      parryWindowDuration(0.25f),
      deathHoldTimer(0.0f),
      deathHoldDuration(3.0f)
{
    position = glm::vec3(0.0f, -0.6f, -2.0f);
    rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    scale = glm::vec3(0.6f);

    // --- Initialize Hitboxes ---
    attackHitboxes.push_back({"mixamorig_Spine2", glm::vec3(0.0f, 150.0f, 0.0f), 1.5f});
    blockHitboxes.push_back({"mixamorig_Spine2", glm::vec3(0.0f, 150.0f, 0.0f), 1.0f});
}

void Player::processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    moveDir = glm::vec3(0.0f);

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
        // Force transition to walk if stamina is depleted
        if (stamina <= 0.0f && charState == RUN) {
            speed = 2.0f;
            charState = RUN_WALK; // Transition back to walk
        } else {
            speed = 3.5f;
        }
        break;
    case BLOCK_WALK: case BLOCK_WALKING:
        speed = 1.75f;
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
        if (!isBlocking) {
            // Don't play block sound here - only when actually blocking an attack
            if (soundEngine) {
                soundEngine->stopSound("footstep");
                soundEngine->stopSound("footstep_fast");
            }
            // Start parry window on first block press
            inParryWindow = true;
            parryWindowStart = glfwGetTime();
        }
        isBlocking = true;
        //charState = BLOCK;
        charState = INIT_BLOCK;
    }
}

void Player::tryDodge()
{
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS && stamina > 0.0f)
    {
        playActionSound("dodge");
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

void Player::consumeStamina(float amount) {
    stamina -= amount;
    if (stamina < 0.0f) {
        stamina = 0.0f;
    }
    lastStaminaUse = glfwGetTime();
}

bool Player::handleAttack(Animation& attackAnim, float damageStart, float damageEnd) {
    chain = false;
    animator.PlayAnimation(&attackAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

    // Damage window
    if (animator.m_CurrentTime > attackAnim.GetDuration() * damageStart && animator.m_CurrentTime < attackAnim.GetDuration() * damageEnd) {
        isDamageActive = true;
    }

    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
        chain = true;
    }

    return animator.m_CurrentTime > attackAnim.GetDuration() - 0.1f;
}

void Player::playActionSound(const std::string& soundName) {
    if (soundEngine) {
        soundEngine->stopSound("footstep");
        soundEngine->stopSound("footstep_fast");
        soundEngine->playSound(soundName);
    }
}

float chainWindowStart = 0.6f;

void Player::update(float deltaTime)
{
    if (charState == DEAD) {
        return; // No further updates if dead
    }

    isDamageActive = false;
    isBlocking = false;
    isTakingHit = false; // Reset taking hit flag each frame

    if (health <= 0) {
        charState = DYING;
    }

    // Update parry window
    if (inParryWindow) {
        float currentTime = glfwGetTime();
        if (currentTime - parryWindowStart > parryWindowDuration) {
            inParryWindow = false; // Parry window expired
        }
    }

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
        if (isMoving()) {
            charState = IDLE_WALK;
        }
        // if left click attack (only if stamina > 0)
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && stamina > 0.0f) {
            charState = IDLE_ATTACK_1;
            hasHitTarget = false; // Reset hit flag for new attack
        }

        break;

    case IDLE_WALK:
        handleAnimationBlend(&idleAnim, &walkAnim, charState, WALK);
        break;

    case WALK:
        currentAnim = &walkAnim;
        if (soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }
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
            hasHitTarget = false; // Reset hit flag for new attack
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
        handleAnimationBlend(&walkAnim, &idleAnim, charState, IDLE);
        break;

    case WALK_ATTACK:
        if (handleAnimationBlend(&walkAnim, &attackAnim1, charState, ATTACK_1)) {
            playActionSound("sword_hit");
            consumeStamina(attackStaminaCost);
        }
        break;


    case WALK_RUN:
        handleAnimationBlend(&walkAnim, &runAnim, charState, RUN);
        break;

    case RUN:
        currentAnim = &runAnim;
        if (soundEngine && !soundEngine->isSoundPlaying("footstep_fast")) {
            soundEngine->playSound("footstep_fast");
        }
        // Only consume stamina when actually moving
        if (glm::length(moveDir) > 0.0f) {
            consumeStamina(15.0f * deltaTime);
        }
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
            hasHitTarget = false; // Reset hit flag for new attack
            if (soundEngine) {
                soundEngine->stopSound("footstep_fast");
            }
        }
        break;

    case RUN_IDLE:
        handleAnimationBlend(&runAnim, &idleAnim, charState, IDLE);
        break;

    case RUN_WALK:
        handleAnimationBlend(&runAnim, &walkAnim, charState, WALK);
        break;

    case RUN_ATTACK:
        if (handleAnimationBlend(&runAnim, &runAttackAnim, charState, RUN_ATTACKING)) {
            playActionSound("sword_hit");
            consumeStamina(runAttackStaminaCost);
        }
        break;

    case RUN_ATTACKING:
        currentAnim = &runAttackAnim;
        if (handleAttack(runAttackAnim, 0.3f, 0.7f)) {
            float startTime = animator.m_CurrentTime2;

            // update character pos first
			position += getForwardDir() * 1.69f;

            if (chain) {
                consumeStamina(attackStaminaCost);
                hasHitTarget = false; // Reset hit flag for chain attack
                animator.PlayAnimation(&runAttackAnim, &attackAnim2, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_2;
            }
            else {
                animator.PlayAnimation(&runAttackAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = RUN_IDLE;
            }
        }
        break;

    case START_DODGE:
        if (handleAnimationBlend(currentAnim, &dodgeAnim, charState, DODGE_END)) {
            // Consume stamina
            consumeStamina(dodgeStaminaCost);
            playActionSound("dodge");
        }
        break;

    case DODGE_END:
        if (animator.m_CurrentTime > 0.7f) {
            if (handleAnimationBlend(&dodgeAnim, &idleAnim, charState, IDLE)) {
                position += getForwardDir() * -0.74f;
            }
        }
        else {
            animator.PlayAnimation(&dodgeAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);
        }
        break;

    case IDLE_ATTACK_1:
        if (handleAnimationBlend(&idleAnim, &attackAnim1, charState, ATTACK_1)) {
            playActionSound("sword_hit");
            consumeStamina(attackStaminaCost);
        }
        break;

    case ATTACK_1:
        currentAnim = &attackAnim1;
		tryBlock();
        tryDodge(); 
        if (handleAttack(attackAnim1, 0.3f, 0.6f)) {
            float startTime = animator.m_CurrentTime2;
            if (chain) {
                consumeStamina(attackStaminaCost);
                hasHitTarget = false; // Reset hit flag for chain attack
                animator.PlayAnimation(&attackAnim1, &attackAnim2, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_2;
            }
            else {
                animator.PlayAnimation(&attackAnim1, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE;
            }
        }
        break;

    case CHAIN_ATTACK_2:
        if (handleAnimationBlend(&attackAnim1, &attackAnim2, charState, ATTACK_2)) {
            playActionSound("sword_hit");
            consumeStamina(attackStaminaCost);
        }
        break;

    case ATTACK_2:
        currentAnim = &attackAnim2;
        tryBlock();
        tryDodge();

        if (handleAttack(attackAnim2, 0.3f, 0.6f)) {
            float startTime = animator.m_CurrentTime2;
            if (chain) {
                consumeStamina(attackStaminaCost);
                hasHitTarget = false; // Reset hit flag for chain attack
                animator.PlayAnimation(&attackAnim2, &attackAnim3, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = CHAIN_ATTACK_3;
            }
            else {
                animator.PlayAnimation(&attackAnim2, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE;
            }
        }
        break;

    case CHAIN_ATTACK_3:
        if (handleAnimationBlend(&attackAnim2, &attackAnim3, charState, ATTACK_3)) {
            playActionSound("sword_hit");
            consumeStamina(attackStaminaCost);
        }
        break;

    case ATTACK_3:
        currentAnim = &attackAnim3;
        if (handleAttack(attackAnim3, 0.4f, 0.7f)) {
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&attackAnim3, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case INIT_BLOCK:
        if (handleAnimationBlend(currentAnim, &initBlockAnim, charState, INIT_BLOCK_TO_BLOCK)) {
            // This block is executed when the blend is complete and state is changed.
        }
        break;

    case INIT_BLOCK_TO_BLOCK:
        handleAnimationBlend(&initBlockAnim, &blockAnim, charState, BLOCK);
        break;

    case INIT_BLOCK_IDLE:
        handleAnimationBlend(&blockAnim, &idleAnim, charState, IDLE);
        break;


    case BLOCK:
        if (soundEngine) {
            soundEngine->stopSound("footstep");
        }
        animator.PlayAnimation(&blockAnim, NULL, animator.m_CurrentTime, 0.0f, 0.0f);

        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
            charState = BLOCK_IDLE;
		}
        else if (isMoving()) {
			charState = BLOCK_WALK;
		}
        break;

    case BLOCK_WALK:
        if (soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }
        handleAnimationBlend(&blockAnim, &blockWalkAnim, charState, BLOCK_WALKING);
        break;

    case BLOCK_WALKING:
        consumeStamina(5.0f * deltaTime);
        if (stamina < 0.0f) {
            stamina = 0.0f;
            isBlocking = false; // Stop blocking when out of stamina
        }
        animator.PlayAnimation(&blockWalkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (soundEngine && !soundEngine->isSoundPlaying("footstep")) {
            soundEngine->playSound("footstep");
        }

        if (!isMoving()) {
            charState = BLOCK;
        }
        else if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&blockWalkAnim, &idleAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            charState = IDLE;
        }
        break;
        

    case BLOCK_IDLE:
        handleAnimationBlend(&blockAnim, &idleAnim, charState, IDLE);
        break;

    case PARRY:
        animator.PlayAnimation(&parryAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (animator.m_CurrentTime > parryAnim.GetDuration() - 0.1f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

    case IS_HIT:
        currentAnim = &isHitAnim;
        // Sound is played when the state is set by the boss collision detection
        animator.PlayAnimation(&isHitAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (animator.m_CurrentTime > isHitAnim.GetDuration() - 0.1f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(&idleAnim, NULL, startTime, 0.0f, blendAmount);
            charState = IDLE;
        }
        break;

case DYING:
        currentAnim = &deadAnim;
        
        if (animator.m_CurrentTime > deadAnim.GetDuration() - 0.1f) {
            if (deathHoldTimer <= 0.0f) {
                deathHoldTimer = glfwGetTime();
            }
            
            // Hold at the last frame - don't play animation anymore
            animator.m_CurrentTime = deadAnim.GetDuration() - 0.05f;
            
            if (glfwGetTime() - deathHoldTimer >= deathHoldDuration) {
                charState = DEAD;
            }
        } else {
            // Only play animation if we haven't reached the end yet
            animator.PlayAnimation(&deadAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        }
        break;
    }

    animator.UpdateAnimation(deltaTime);

    // --- Update Hitbox Positions ---
    updateHitboxes(attackHitboxes);
    updateHitboxes(blockHitboxes);
}

bool Player::isAttacking() const {
    return charState == ATTACK_1 || charState == ATTACK_2 || charState == ATTACK_3 ||
           charState == IDLE_ATTACK_1 || charState == CHAIN_ATTACK_2 || charState == CHAIN_ATTACK_3 ||
           charState == RUN_ATTACK || charState == RUN_ATTACKING || charState == WALK_ATTACK;
}

bool Player::isBlockingState() const {
    return charState == INIT_BLOCK || charState == INIT_BLOCK_TO_BLOCK || charState == BLOCK || charState == BLOCK_WALK || charState == BLOCK_IDLE || charState == BLOCK_WALKING || isBlocking;
}

bool Player::isMoving() const {
    return glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS ||
           glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS;
}

bool Player::isInParryWindow() const {
    return inParryWindow;
}

bool Player::isDead() const {
    return charState == DEAD;
}

void Player::checkCollisionWithBoss(Boss& boss) {
    if (boss.isDead()) return;
    
    // Player attacking boss
    if (isDamageActive && isAttacking() && !hasHitTarget) {
        for (const auto& playerHitbox : attackHitboxes) {
            float distanceToBoss = glm::distance(playerHitbox.worldPosition, boss.position);
            if (distanceToBoss < playerHitbox.radius + 0.8f) { // 0.8f is boss body radius
                boss.takeDamage(25); // Player deals 25 damage
                hasHitTarget = true; // Mark that we've hit something with this attack
                break; // Only hit once per attack
            }
        }
    }
}
