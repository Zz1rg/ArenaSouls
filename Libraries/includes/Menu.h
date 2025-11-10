#pragma once

#include <learnopengl/shader_m.h>
#include <GLFW/glfw3.h>

class Menu {
public:
    Menu();
    void draw();
    bool isStartButtonClicked(double xpos, double ypos);

private:
    Shader menuShader;
    unsigned int menuVAO, menuVBO;
    unsigned int backgroundTexture;

    unsigned int loadTexture(const char* path);
};
