#pragma once

#include <glm/glm.hpp>
#include <learnopengl/model_animation.h>
#include <learnopengl/animator.h>
#include <learnopengl/shader_m.h>
#include "Hitbox.h"
#include <vector>

class BaseEntity {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Model model;
    Animator animator;
    
    float blendAmount;
    float blendRate;

    bool isDamageActive;
    std::vector<Hitbox> attackHitboxes;
    std::vector<Hitbox> blockHitboxes;

    BaseEntity(const std::string& modelPath, Animation* initialAnimation);

    virtual ~BaseEntity() = default;

    virtual void update(float deltaTime) = 0;

    virtual void draw(Shader& shader);

    void updateHitboxes(std::vector<Hitbox>& hitboxes);

protected:
    template<typename StateEnum>
    bool handleAnimationBlend(Animation* from, Animation* to, StateEnum& currentState, StateEnum nextState) {
        blendAmount += blendRate;
        blendAmount = fmod(blendAmount, 1.0f);
        animator.PlayAnimation(from, to, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
        if (blendAmount >= 0.9f) {
            blendAmount = 0.0f;
            float startTime = animator.m_CurrentTime2;
            animator.PlayAnimation(to, NULL, startTime, 0.0f, blendAmount);
            currentState = nextState;
            return true;
        }
        return false;
    }
};
