#ifndef UI_H
#define UI_H

#include <glad/glad.h>
#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class UI {
private:
    unsigned int VAO, VBO;
    Shader* uiShader;
    glm::mat4 projection;

public:
    UI();
    ~UI();
    void initialize(int screenWidth, int screenHeight);
    void renderHealthBar(float health, float maxHealth);
    void renderStaminaBar(float stamina, float maxStamina);
    void cleanup();

private:
    void renderRectangle(float x, float y, float width, float height, glm::vec3 color);
};

#endif