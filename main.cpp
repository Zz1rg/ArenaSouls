#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader_m.h>
#include "PlayerCamera.h"
#include "Player.h"
#include "Menu.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/model.h>
#include "TrainingDummy.h"

enum GameState {
    GAME_MENU,
    GAME_ACTIVE
};

GameState State = GAME_MENU;

struct Application {
    Menu* menu;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

bool left_mouse_button_pressed = false;
bool right_mouse_button_pressed = false;

// Camera and timing
PlayerCamera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// wall collision
const float WALL_X = 17.5f;
const float WALL_Z = 17.5f;

// character position for camera raycasting
glm::vec3 charPosition(0.0f, 0.0f, 0.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ArenaSouls", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Menu menu;
    Application app;
    app.menu = &menu;
    glfwSetWindowUserPointer(window, &app);

    stbi_set_flip_vertically_on_load(true);

    // build and compile shaders
    // -------------------------
    Shader shader("anim_model.vs", "anim_model.fs");
    Player player;
    TrainingDummy dummy; // Position dummy 10 units ahead
    //Model arena("resources/objects/arena/maze-grass/obj_export/maze_grass.obj");
    Model arena("resources/objects/arena/obj_v3/arena.obj");

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (State == GAME_ACTIVE)
        {
            glEnable(GL_DEPTH_TEST);
            player.update(deltaTime);
            player.processInput(window, camera, deltaTime);
            if (abs(player.position.x) > WALL_X)
            {
                player.position.x = (player.position.x > 0) ? WALL_X : -WALL_X;
            }
            if (abs(player.position.z) > WALL_Z)
            {
                player.position.z = (player.position.z > 0) ? WALL_Z : -WALL_Z;
            }
            charPosition = player.position;
            //camera.FollowPlayer(player.position);
            dummy.update(deltaTime, player);

            glClearColor(0.817f, 0.9529f, 0.9804f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 1000.0f / 800.0f, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            
            shader.setVec3("viewPos", camera.Position);
            shader.setVec3("lightPos", glm::vec3(4.0f, 4.0f, 4.0f));
            //std::cout << "HI";

            shader.setBool("useBones", true);
            player.draw(shader);
            dummy.draw(shader);

            // draw arena
            shader.setBool("useBones", false);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.35f, 0.0f)); // Translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f)); // Scale it down
            shader.setMat4("model", model);
            arena.Draw(shader);

            glm::vec3 cameraOffset(0.0f, 1.5f, 3.0f); // Adjusted height and distance
            float panSpeed = 0.5f; // Smooth horizontal panning speed
            float verticalPanSpeed = 0.5f; // Smooth vertical panning speed
            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                camera.shake(0.2f, 0.1f);
            }
            camera.updateShake(deltaTime);
            camera.FollowPlayerWithOffset(player.position, cameraOffset, panSpeed, verticalPanSpeed, deltaTime);
            //camera.FollowPlayerSmoothSpring(player.position, cameraOffset, deltaTime);

            camera.updateShake(deltaTime);
        }
        else if (State == GAME_MENU)
        {
            glDisable(GL_DEPTH_TEST);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            menu.draw();
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
        //std::cout << "Left click: " << left_mouse_button_pressed << " Right click: " << right_mouse_button_pressed << std::endl;
        //std::cout << "Player position: (" << player.position.x << ", " << player.position.y << ", " << player.position.z << ")\n";
        std::cout << "CharState: " << player.charState << std::endl;
        //std::cout << "is blocking: " << player.isBlocking << std::endl;
        //std::cout << "blend amount: " << player.blendAmount << std::endl;
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (State == GAME_ACTIVE)
    {
        camera.ProcessMouseCallback(xpos, ypos);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (State == GAME_MENU)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (app->menu->isStartButtonClicked(xpos, ypos))
            {
                State = GAME_ACTIVE;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
    else if (State == GAME_ACTIVE)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // Handle left mouse button press
            left_mouse_button_pressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            // Handle left mouse button release
            left_mouse_button_pressed = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            // Handle left mouse button press
            right_mouse_button_pressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            // Handle left mouse button release
            right_mouse_button_pressed = false;
        }
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (State == GAME_ACTIVE)
    {
        camera.ProcessScrollCallback(yoffset);
    }
}