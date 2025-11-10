#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/animator.h>
#include <learnopengl/shader_m.h>

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
	WALK_ATTACK,
    ANY_BLOCK, BLOCK, BLOCK_ATTACK, PARRY
};

class Player {
public:
    glm::vec3 position;
    float blendAmount;
    float blendRate;
    AnimState charState;

    Model model;
    Animation idleAnim, walkAnim, runAnim, rollAnim, attackAnim1, attackAnim2, attackAnim3, runAttackAnim, initBlockAnim, blockAnim, parryAnim;
    Animator animator;
    bool chain;

    Player();
    void processInput(GLFWwindow* window, float deltaTime);
    void update(float deltaTime);
    void draw(Shader& shader);
	void tryBlock(Animation &transitAnim);
    //glm::vec3 getPosition();
};

#endif
