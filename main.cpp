#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include "Player.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/model.h>
#include "TrainingDummy.h"

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool left_mouse_button_pressed = false;
bool right_mouse_button_pressed = false;

// Camera and timing
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;
float lastX = 1000.0f / 2.0f, lastY = 800.0f / 2.0f;
bool firstMouse = true;

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// wall collision
const float WALL_X = 17.5f;
const float WALL_Z = 17.5f;

// character position for camera raycasting
glm::vec3 charPosition(0.0f, 0.0f, 0.0f);

float cameraRaycast(const glm::vec3& start, const glm::vec3& end)
{
    glm::vec3 dir = end - start;
    float totalDist = glm::length(dir);
    if (totalDist < 0.001f)
        return totalDist;

    dir = glm::normalize(dir);
    float step = 0.1f;
    float dist = 0.0f;
    return totalDist; // clear line
}

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    Shader shader("anim_model.vs", "anim_model.fs");
    Player player;
    //Model arena("resources/objects/arena/maze-grass/obj_export/maze_grass.obj");
    Model arena("resources/objects/arena/obj_v3/arena.obj");
    TrainingDummy dummy(glm::vec3(0.0f, -0.6f, -10.0f)); // Position dummy 10 units ahead

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
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
        dummy.update(deltaTime, player.position);

        glClearColor(0.817f, 0.9529f, 0.9804f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 1000.0f / 800.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        //std::cout << "HI";

        shader.setBool("useBones", true);
        player.draw(shader);

        // draw arena
        shader.setBool("useBones", false);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.35f, 0.0f)); // Translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f)); // Scale it down
        shader.setMat4("model", model);
        arena.Draw(shader);
        dummy.draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
        //std::cout << "Left click: " << left_mouse_button_pressed << " Right click: " << right_mouse_button_pressed << std::endl;
        //std::cout << "Player position: (" << player.position.x << ", " << player.position.y << ", " << player.position.z << ")\n";
        //std::cout << "CharState: " << player.charState << std::endl;
        //std::cout << "is blocking: " << player.isBlocking << std::endl;
        //std::cout << "blend amount: " << player.blendAmount << std::endl;
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 moveDir(0.0f);
    bool moved = false;

    // Determine movement direction using camera's orientation
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDir += camera.Front;
    moved = true;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir -= camera.Front;
    moved = true;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir -= camera.Right;
    moved = true;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir += camera.Right;
    moved = true;

    //if (!moved) return;

    // Normalize to avoid faster diagonal movement
    if (glm::length(moveDir) > 0.0f)
        moveDir = glm::normalize(moveDir);

    // Update camera to stay behind character
    glm::vec3 offset(0.0f, 0.6f, 1.5f); // height and distance
    glm::vec3 desiredCamPos = charPosition - camera.Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);

    glm::vec3 correctedCamPos = desiredCamPos;

    camera.Position = correctedCamPos;

    //// Perform raycast to check if something blocks the camera
    //float hitDist = cameraRaycast(charPosition + glm::vec3(0.0f, offset.y, 0.0f), desiredCamPos, mazeGrid, maze_grid_size);

    //// If blocked, move camera closer
    //if (hitDist < glm::length(desiredCamPos - charPosition))
    //{
    //	glm::vec3 dir = glm::normalize(desiredCamPos - charPosition);
    //	glm::vec3 newCamPos = charPosition + dir * hitDist;
    //	camera.Position = newCamPos;
    //}
    //else
    //{
    //	camera.Position = desiredCamPos;
    //}
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
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
