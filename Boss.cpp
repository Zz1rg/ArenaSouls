#include "Boss.h"
#include "SoundEngine.h"
#include <stb_image.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <algorithm>

Boss::Boss()
    : BaseEntity("resources/objects/mutant-boss/Mutant Breathing Idle/Mutant Breathing Idle.dae", &idleAnim),
    health(300),
    maxHealth(300),
    moveSpeed(1.5f),
    attackRange(2.0f),
    detectionRange(8.0f),
    idleAnim("resources/objects/mutant-boss/Mutant Breathing Idle/Mutant Breathing Idle.dae", &model),
    walkAnim("resources/objects/mutant-boss/Mutant Walking/Mutant Walking.dae", &model),
    runAnim("resources/objects/mutant-boss/Mutant Run/Mutant Run.dae", &model),
    attack1Anim("resources/objects/mutant-boss/Attack1/Mutant Punch.dae", &model),
    attack2Anim("resources/objects/mutant-boss/Attack2/Mutant Swiping.dae", &model),
    attack3Anim("resources/objects/mutant-boss/Attack3/Zombie Punching.dae", &model),
    stuntAnim("resources/objects/mutant-boss/Stunt/Sword And Shield Impact.dae", &model),
    dyingAnim("resources/objects/mutant-boss/Mutant Dying/Mutant Dying.dae", &model),
    bossState(BOSS_IDLE),
    isDead(false),
    canAttack(true),
    lastAttackTime(0.0f),
    attackCooldown(2.0f),
    soundEngine(nullptr)
{
    position = glm::vec3(5.0f, -0.6f, 5.0f); // Start position away from player
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = glm::vec3(0.8f); // Slightly larger than player

    // Initialize hitboxes for boss attacks
    attackHitboxes.push_back({"mixamorig_RightHand", glm::vec3(0.0f, 0.0f, 0.0f), 0.3f});
    attackHitboxes.push_back({"mixamorig_LeftHand", glm::vec3(0.0f, 0.0f, 0.0f), 0.3f});

    targetPosition = glm::vec3(0.0f);
    currentAnim = &idleAnim;
}

void Boss::update(float deltaTime)
{
    isDamageActive = false;

    if (isDead) {
        return; // Don't update if boss is dead
    }

    // Update attack cooldown
    if (!canAttack) {
        if (glfwGetTime() - lastAttackTime > attackCooldown) {
            canAttack = true;
        }
    }

    switch (bossState) {
    case BOSS_IDLE:
        currentAnim = &idleAnim;
        animator.PlayAnimation(&idleAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        updateAI(deltaTime);
        break;

    case BOSS_WALKING:
        currentAnim = &walkAnim;
        animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        moveTowardsTarget(deltaTime);
        updateAI(deltaTime);
        break;

    case BOSS_RUNNING:
        currentAnim = &runAnim;
        animator.PlayAnimation(&runAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        moveTowardsTarget(deltaTime * 1.5f); // Run faster than walk
        updateAI(deltaTime);
        break;

    case BOSS_ATTACK_1:
        currentAnim = &attack1Anim;
        animator.PlayAnimation(&attack1Anim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        
        // Damage window for attack 1
        if (animator.m_CurrentTime > attack1Anim.GetDuration() * 0.3f && 
            animator.m_CurrentTime < attack1Anim.GetDuration() * 0.7f) {
            isDamageActive = true;
        }

        if (animator.m_CurrentTime > attack1Anim.GetDuration() - 0.1f) {
            bossState = BOSS_IDLE;
            lastAttackTime = glfwGetTime();
            canAttack = false;
        }
        break;

    case BOSS_ATTACK_2:
        currentAnim = &attack2Anim;
        animator.PlayAnimation(&attack2Anim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        
        // Damage window for attack 2
        if (animator.m_CurrentTime > attack2Anim.GetDuration() * 0.4f && 
            animator.m_CurrentTime < attack2Anim.GetDuration() * 0.8f) {
            isDamageActive = true;
        }

        if (animator.m_CurrentTime > attack2Anim.GetDuration() - 0.1f) {
            bossState = BOSS_IDLE;
            lastAttackTime = glfwGetTime();
            canAttack = false;
        }
        break;

    case BOSS_ATTACK_3:
        currentAnim = &attack3Anim;
        animator.PlayAnimation(&attack3Anim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        
        // Damage window for attack 3
        if (animator.m_CurrentTime > attack3Anim.GetDuration() * 0.2f && 
            animator.m_CurrentTime < attack3Anim.GetDuration() * 0.6f) {
            isDamageActive = true;
        }

        if (animator.m_CurrentTime > attack3Anim.GetDuration() - 0.1f) {
            bossState = BOSS_IDLE;
            lastAttackTime = glfwGetTime();
            canAttack = false;
        }
        break;

    case BOSS_STUNT:
        currentAnim = &stuntAnim;
        animator.PlayAnimation(&stuntAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        
        if (animator.m_CurrentTime > stuntAnim.GetDuration() - 0.1f) {
            bossState = BOSS_IDLE;
        }
        break;

    case BOSS_DYING:
        currentAnim = &dyingAnim;
        animator.PlayAnimation(&dyingAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        
        if (animator.m_CurrentTime > dyingAnim.GetDuration() - 0.1f) {
            bossState = BOSS_DEAD;
            isDead = true;
            // playActionSound("boss_death");
        }
        break;

    case BOSS_DEAD:
        // Boss is dead, no animation updates needed
        return;
    }

    animator.UpdateAnimation(deltaTime);

    // Update hitbox positions
    updateHitboxes(attackHitboxes);
}

void Boss::updateAI(float deltaTime)
{
    if (bossState == BOSS_ATTACK_1 || bossState == BOSS_ATTACK_2 || 
        bossState == BOSS_ATTACK_3 || bossState == BOSS_STUNT || 
        bossState == BOSS_DYING || bossState == BOSS_DEAD) {
        return; // Don't update AI during these states
    }

    float distanceToTarget = getDistanceToTarget();

    // Check if player is in detection range
    if (distanceToTarget > detectionRange) {
        bossState = BOSS_IDLE;
        return;
    }

    // Check if player is in attack range and boss can attack
    if (distanceToTarget <= attackRange && canAttack) {
        performAttack();
        return;
    }

    // Move towards player
    if (distanceToTarget > attackRange) {
        // Rotate to face target
        glm::vec3 direction = glm::normalize(targetPosition - position);
        float targetYaw = glm::degrees(atan2(direction.x, direction.z));
        rotation.y = targetYaw;

        // Choose movement state based on distance
        if (distanceToTarget > detectionRange * 0.6f) {
            bossState = BOSS_RUNNING;
        } else {
            bossState = BOSS_WALKING;
        }
    }
}

void Boss::moveTowardsTarget(float deltaTime)
{
    if (targetPosition == glm::vec3(0.0f)) return;

    glm::vec3 direction = glm::normalize(targetPosition - position);
    position += direction * moveSpeed * deltaTime;
}

void Boss::performAttack()
{
    // Randomly choose an attack
    int attackChoice = rand() % 3;
    
    switch (attackChoice) {
    case 0:
        bossState = BOSS_ATTACK_1;
        // playActionSound("boss_attack1");
        break;
    case 1:
        bossState = BOSS_ATTACK_2;
        // playActionSound("boss_attack2");
        break;
    case 2:
        bossState = BOSS_ATTACK_3;
        // playActionSound("boss_attack3");
        break;
    }
    
    // Reset animation time for new attack
    animator.m_CurrentTime = 0.0f;
}

void Boss::setTarget(const glm::vec3& playerPos)
{
    targetPosition = playerPos;
}

void Boss::takeDamage(int damage)
{
    if (isDead) return;

    health -= damage;
    if (health <= 0) {
        health = 0;
        bossState = BOSS_DYING;
        animator.m_CurrentTime = 0.0f;
        // playActionSound("boss_hurt");
    } else {
        // Play stunt animation occasionally when taking damage
        if (rand() % 3 == 0) { // 33% chance to stunt
            bossState = BOSS_STUNT;
            animator.m_CurrentTime = 0.0f;
            // playActionSound("boss_hurt");
        }
    }
}

void Boss::setSoundEngine(SoundEngine* engine)
{
    soundEngine = engine;
}

bool Boss::isAttacking() const
{
    return bossState == BOSS_ATTACK_1 || bossState == BOSS_ATTACK_2 || bossState == BOSS_ATTACK_3;
}

bool Boss::isAlive() const
{
    return !isDead && health > 0;
}

float Boss::getDistanceToTarget() const
{
    if (targetPosition == glm::vec3(0.0f)) return 999.0f; // Very far if no target
    return glm::distance(position, targetPosition);
}

void Boss::playActionSound(const std::string& soundName)
{
    if (soundEngine) {
        soundEngine->playSound(soundName);
    }
}

glm::vec3 Boss::getForwardDir()
{
    return glm::normalize(glm::vec3(
        sin(glm::radians(rotation.y)),
        0.0f,
        cos(glm::radians(rotation.y))
    ));
}