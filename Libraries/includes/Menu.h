#pragma once

#include <learnopengl/shader_m.h>
#include <GLFW/glfw3.h>

class Menu {
public:
    Menu();
    void draw();
    void drawWinMenu();
    void drawLoseMenu();
    bool isStartButtonClicked(double xpos, double ypos);
    bool isRestartButtonClicked(double xpos, double ypos);

private:
    Shader menuShader;
    unsigned int menuVAO, menuVBO;
    unsigned int backgroundTexture;
    unsigned int win_backgroundTexture;
    unsigned int lose_backgroundTexture;
    float winOpacity;
    float loseOpacity;

    unsigned int loadTexture(const char* path);
};
