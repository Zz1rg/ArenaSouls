#include "Menu.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>

float menuVertices[] = {
    // positions     // texture coords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

Menu::Menu() : menuShader("menu.vs", "menu.fs") {
    stbi_set_flip_vertically_on_load(true);
    backgroundTexture = loadTexture("resources/background.jpg");
    win_backgroundTexture = loadTexture("resources/win_background.jpg");
    lose_backgroundTexture = loadTexture("resources/lose_background.jpg");
    
    winOpacity = 1.0f;
    loseOpacity = 1.0f;
    
    menuShader.use();
    menuShader.setInt("background", 0);

    glGenVertexArrays(1, &menuVAO);
    glGenBuffers(1, &menuVBO);
    glBindVertexArray(menuVAO);
    glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(menuVertices), menuVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    stbi_set_flip_vertically_on_load(false);
}

void Menu::draw() {
    menuShader.use();
    menuShader.setFloat("opacity", 1.0f); // Full opacity for main menu
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBindVertexArray(menuVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool Menu::isStartButtonClicked(double xpos, double ypos) {
    // Start button area (example coordinates)
    return (xpos > 400 && xpos < 600 && ypos > 350 && ypos < 450);
}

void Menu::drawWinMenu() {
    // Draw the background first
    menuShader.use();
    menuShader.setFloat("opacity", winOpacity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, win_backgroundTexture);
    glBindVertexArray(menuVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Here you would render "Victory!" or "You Win!" text
    // And options like "Play Again", "Main Menu"
}

void Menu::drawLoseMenu() {
    // Draw the background first
    menuShader.use();
    menuShader.setFloat("opacity", loseOpacity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lose_backgroundTexture);
    glBindVertexArray(menuVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Here you would render "Defeat!" or "You Died!" text
    // And options like "Try Again", "Main Menu"
}

bool Menu::isRestartButtonClicked(double xpos, double ypos) {
    // Restart button area (positioned above main menu button)
    return (xpos > 430 && xpos < 564 && ypos > 515 && ypos < 566);
}

unsigned int Menu::loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}