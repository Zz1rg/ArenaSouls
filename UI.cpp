#include "UI.h"

UI::UI() : VAO(0), VBO(0), uiShader(nullptr) {
}

UI::~UI() {
    cleanup();
}

void UI::initialize(int screenWidth, int screenHeight) {
    // Create and compile shader
    uiShader = new Shader("ui.vs", "ui.fs");
    
    // Set up orthographic projection matrix for 2D rendering
    projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    
    // Create vertex array and buffer for rectangle rendering
    float vertices[] = {
        0.0f, 1.0f,  // top left
        1.0f, 1.0f,  // top right
        0.0f, 0.0f,  // bottom left
        1.0f, 0.0f   // bottom right
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UI::renderHealthBar(float health, float maxHealth) {
    float barWidth = 500.0f;
    float barHeight = 20.0f;
    float x = 20.0f;
    float y = 20.0f;
    
    // Render background (dark gray)
    renderRectangle(x, y, barWidth, barHeight, glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Render health bar (green)
    float healthPercent = health / maxHealth;
    if (healthPercent > 0.0f) {
        renderRectangle(x, y, barWidth * healthPercent, barHeight, glm::vec3(0.0f, 0.8f, 0.0f));
    }
}

void UI::renderStaminaBar(float stamina, float maxStamina) {
    float barWidth = 300.0f;
    float barHeight = 20.0f;
    float x = 20.0f;
    float y = 50.0f; // Below health bar
    
    // Render background (dark gray)
    renderRectangle(x, y, barWidth, barHeight, glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Render stamina bar (yellow to orange gradient effect - using yellow for now)
    float staminaPercent = stamina / maxStamina;
    if (staminaPercent > 0.0f) {
        // Create color gradient from yellow (high stamina) to orange (low stamina)
        glm::vec3 color;
        if (staminaPercent > 0.5f) {
            // Yellow to orange-yellow
            color = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
        } else {
            // Orange-yellow to orange
            float t = staminaPercent * 2.0f; // Map 0-0.5 to 0-1
            color = glm::vec3(1.0f, 0.5f + t * 0.5f, 0.0f); // Orange to yellow-orange
        }
        renderRectangle(x, y, barWidth * staminaPercent, barHeight, color);
    }
}

void UI::renderBossHealthBar(float health, float maxHealth, const std::string& bossName) {
    float barWidth = 600.0f;
    float barHeight = 30.0f;
    float x = 200.0f; // Center it more on screen
    float y = 750.0f;
    
    // Render background (dark red)
    renderRectangle(x, y, barWidth, barHeight, glm::vec3(0.3f, 0.1f, 0.1f));
    
    // Render boss health bar (red)
    float healthPercent = health / maxHealth;
    if (healthPercent > 0.0f) {
        // Create color gradient from red (low health) to dark red (high health)
        glm::vec3 color;
        if (healthPercent > 0.3f) {
            color = glm::vec3(0.8f, 0.1f, 0.1f); // Dark red
        } else {
            color = glm::vec3(1.0f, 0.0f, 0.0f); // Bright red (low health warning)
        }
        renderRectangle(x, y, barWidth * healthPercent, barHeight, color);
    }
}

void UI::renderRectangle(float x, float y, float width, float height, glm::vec3 color) {
    if (!uiShader) return;
    
    uiShader->use();
    
    // Create model matrix for positioning and scaling
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    
    uiShader->setMat4("projection", projection);
    uiShader->setMat4("model", model);
    uiShader->setVec3("uColor", color);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void UI::cleanup() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (uiShader) {
        delete uiShader;
        uiShader = nullptr;
    }
}