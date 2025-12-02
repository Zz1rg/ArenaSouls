#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include <GLFW/glfw3.h>
#include <learnopengl/camera.h>
#include "PlayerCamera.h"


// Forward declaration
class SoundEngine;

enum AnimState {
    IDLE = 1,
    IDLE_WALK, WALK_IDLE, WALK,
    RUN, IDLE_RUN, RUN_IDLE,
	ATTACK_1, ATTACK_2, ATTACK_3,
    IDLE_ATTACK_1, ATTACK_1_IDLE,
    CHAIN_ATTACK_2, ATTACK_2_IDLE,
    CHAIN_ATTACK_3, ATTACK_3_IDLE,
    START_DODGE, DODGE_END,
    RUN_ATTACK,
    RUN_ATTACKING,
	WALK_ATTACK, WALK_RUN, RUN_WALK,
	INIT_BLOCK, INIT_BLOCKING, INIT_BLOCK_TO_BLOCK, INIT_BLOCK_IDLE,
    BLOCK, BLOCK_IDLE, PARRY, BLOCK_WALK, BLOCK_WALKING,
    IS_HIT,
    DYING, DEAD
};

class Player : public BaseEntity {
public:
    AnimState charState;

    int health;
    float stamina;
    int maxHealth;
    float maxStamina;
    float staminaRegenRate;
    float staminaRegenDelay;
    float lastStaminaUse;

    Animation idleAnim, walkAnim, runAnim, dodgeAnim, attackAnim1, attackAnim2, attackAnim3, runAttackAnim, initBlockAnim, blockAnim, parryAnim, blockWalkAnim, isHitAnim, deadAnim;
    Animation *currentAnim;
    
    bool chain;
	bool isBlocking;
    SoundEngine* soundEngine;
    PlayerCamera* cameraRef; // Reference to camera for shake effects
    glm::vec3 moveDir;
    bool hasHitTarget;   // Track if current attack has already hit a target
    bool isTakingHit;    // Track if player is currently taking damage
    bool inParryWindow;  // Track if player is in the parry timing window
    float parryWindowStart; // When the parry window started
    float parryWindowDuration; // How long the parry window lasts (0.25s)

    Player();
    void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
    void update(float deltaTime) override;
    void tryBlock();
    void tryDodge();
    void updateStamina(float deltaTime);
    void setSoundEngine(SoundEngine* engine);
    void setCameraReference(class PlayerCamera* camera);
    glm::vec3 getForwardDir();
    void checkCollisionWithBoss(class Boss& boss); // Forward declaration collision method

    bool isAttacking() const;
    bool isBlockingState() const;
    bool isDead() const;
    bool isMoving() const;
    bool isInParryWindow() const;
    void consumeStamina(float amount);

private:
    bool handleAttack(Animation& attackAnim, float damageStart, float damageEnd);
    void playActionSound(const std::string& soundName);
    float deathHoldTimer;
    float deathHoldDuration;
};

#endif
