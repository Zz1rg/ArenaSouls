#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/animator.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

enum AnimState {
    IDLE = 1,
    IDLE_WALK, WALK_IDLE, WALK,
    RUN, IDLE_RUN, RUN_IDLE,
	ATTACK_1, ATTACK_2, ATTACK_3,
    IDLE_ATTACK_1, ATTACK_1_IDLE,
    CHAIN_ATTACK_2, ATTACK_2_IDLE,
    CHAIN_ATTACK_3, ATTACK_3_IDLE,
    ROLL_IDLE,
    IDLE_ROLL,
    RUN_ATTACK,
    RUN_ATTACKING,
	WALK_ATTACK, WALK_RUN, RUN_WALK,
	INIT_BLOCK, INIT_BLOCK_TO_BLOCK, BLOCK, BLOCK_IDLE, PARRY, BLOCK_WALK, BLOCK_WALKING
};

class Player {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float blendAmount;
    float blendRate;
    AnimState charState;

    Model model;
    Animation idleAnim, walkAnim, runAnim, rollAnim, attackAnim1, attackAnim2, attackAnim3, runAttackAnim, initBlockAnim, blockAnim, parryAnim, blockWalkAnim;
    Animator animator;
    bool chain;
	bool isBlocking;
    float currentSpeed;

    Player();
    void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
    void update(float deltaTime);
    void draw(Shader& shader);
	void tryBlock(Animation &transitAnim);
    glm::vec3 getForwardDir();
    //glm::vec3 getPosition();

    // Check if the player is currently attacking
    bool isAttacking() const;

    // Check if the player is in a blocking state
    bool isBlockingState() const;

    bool isMoving() const;
};

#endif
