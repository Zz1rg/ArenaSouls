#pragma once

#include <learnopengl/shader_m.h>
#include <learnopengl/model_animation.h>
#include <glm/glm.hpp>
#include <vector>

class DebugDrawer {
public:
    DebugDrawer();
    ~DebugDrawer();

    void drawSphere(const glm::vec3& position, float radius, const glm::vec3& color, const glm::mat4& view, const glm::mat4& projection);

private:
    Shader debugShader;
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    unsigned int sphereIndexCount;

    void setupSphere();
};
