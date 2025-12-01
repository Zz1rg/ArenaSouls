#ifndef BOSS_H
#define BOSS_H

#include "Entity.h"
#include <GLFW/glfw3.h>
#include <learnopengl/camera.h>

// Forward declaration
class SoundEngine;

enum BossState {
    BOSS_IDLE = 1,
    BOSS_WALKING,
    BOSS_RUNNING,
    BOSS_ATTACK_1,
    BOSS_ATTACK_2,
    BOSS_ATTACK_3,
    BOSS_STUNT,
    BOSS_LONG_STUNT,
    BOSS_DYING,
    BOSS_DEAD
};

class Boss : public BaseEntity {
public:
    BossState bossState;

    int health;
    int maxHealth;
    float moveSpeed;
    float attackRange;
    float detectionRange;

    Animation idleAnim, walkAnim, runAnim, attack1Anim, attack2Anim, attack3Anim, stuntAnim, longStuntAnim, dyingAnim;
    Animation* currentAnim;
    
    SoundEngine* soundEngine;
    glm::vec3 targetPosition; // Player position for AI targeting
    bool isDead;
    bool canAttack;
    float lastAttackTime;
    float attackCooldown;
    bool hasHitPlayer;  // Track if current attack has already hit player
    bool isTakingHit;   // Track if boss is currently taking damage

    Boss();
    void update(float deltaTime) override;
    void setTarget(const glm::vec3& playerPos);
    void takeDamage(int damage);
    void setSoundEngine(SoundEngine* engine);
    void checkCollisionWithPlayer(class Player& player); // Forward declaration collision method
    
    bool isAttacking() const;
    bool isAlive() const;
    float getDistanceToTarget() const;

private:
    void updateAI(float deltaTime);
    void moveTowardsTarget(float deltaTime);
    void performAttack();
    void playActionSound(const std::string& soundName);
    glm::vec3 getForwardDir();
};

#endif