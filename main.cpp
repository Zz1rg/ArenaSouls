#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader_m.h>
#include "PlayerCamera.h"
#include "Player.h"
#include "Boss.h"
#include "Menu.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/model.h>
#include "TrainingDummy.h"
#include "UI.h"
#include "SoundEngine.h"
#include "DebugDrawer.h"
#include <vector>

enum GameState {
    GAME_MENU,
    GAME_STARTING,
    GAME_ACTIVE,
    GAME_WIN,
    GAME_LOSE
};

GameState State = GAME_MENU;

struct Application {
    Menu* menu;
    Player* player;
    Boss* boss;
    TrainingDummy* dummy;
    PlayerCamera* camera;
    SoundEngine* soundEngine;
    
    void resetGame();
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);
void initGame(Player& player, Boss& boss, TrainingDummy& dummy, PlayerCamera& camera, SoundEngine& soundEngine);

// Camera and timing
PlayerCamera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// wall collision
const float MAX_WALL_X = 17.4f;
const float MIN_WALL_X = -10.0f;
const float MAX_WALL_Z = 13.1f;
const float MIN_WALL_Z = -23.4f;

// UI system
UI ui;

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

    // Initialize UI system
    ui.initialize(SCR_WIDTH, SCR_HEIGHT);

    // Initialize Sound Engine
    SoundEngine soundEngine;
    soundEngine.initialize();
    
    // Initialize Debug Drawer
    DebugDrawer debugDrawer;
    
    // build and compile shaders
    // -------------------------
    Shader shader("anim_model.vs", "anim_model.fs");
    Player player;
    player.setSoundEngine(&soundEngine);
    player.setCameraReference(&camera);
    Boss boss;
    boss.setSoundEngine(&soundEngine);
    boss.setPlayerReference(&player); // Set player reference for death checking
    TrainingDummy dummy; // Position dummy 10 units ahead
    //Model arena("resources/objects/arena/maze-grass/obj_export/maze_grass.obj");
    Model arena("resources/objects/arena/obj_v3/arena.obj");
    
    // Set up application references
    app.player = &player;
    app.boss = &boss;
    app.dummy = &dummy;
    app.camera = &camera;
    app.soundEngine = &soundEngine;
    
    // Initialize game state
    initGame(player, boss, dummy, camera, soundEngine);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && State == GAME_STARTING) {
            State = GAME_ACTIVE;
        }

        if (State == GAME_ACTIVE)
        {
            if (player.isDead()) {
                State = GAME_LOSE;
            } else if (boss.isDead()) {
                State = GAME_WIN;
            }

            // Switch to battle music
            if (soundEngine.getCurrentMusic() != "battle") {
                soundEngine.switchToMusic("battle");
            }
            
            glEnable(GL_DEPTH_TEST);
            player.update(deltaTime);
            player.processInput(window, camera, deltaTime);
            if (player.position.x > MAX_WALL_X) player.position.x = MAX_WALL_X;
            if (player.position.x < MIN_WALL_X) player.position.x = MIN_WALL_X;
            if (player.position.z > MAX_WALL_Z) player.position.z = MAX_WALL_Z;
            if (player.position.z < MIN_WALL_Z) player.position.z = MIN_WALL_Z;
            
            // Update boss AI with player position
            boss.setTarget(player.position);
            boss.update(deltaTime);
            
            // Boss collision with walls
            if (boss.position.x > MAX_WALL_X) boss.position.x = MAX_WALL_X;
            if (boss.position.x < MIN_WALL_X) boss.position.x = MIN_WALL_X;
            if (boss.position.z > MAX_WALL_Z) boss.position.z = MAX_WALL_Z;
            if (boss.position.z < MIN_WALL_Z) boss.position.z = MIN_WALL_Z;
            
            // Check collisions between player and boss
            boss.checkCollisionWithPlayer(player);
            player.checkCollisionWithBoss(boss);
            
            // dummy.update(deltaTime, player);

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
            boss.draw(shader);
            // dummy.draw(shader);

            //std::cout << "Player Position: (" << player.position.x << ", " << player.position.y << ", " << player.position.z << ")\n";

            // draw arena
            shader.setBool("useBones", false);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.35f, 0.0f)); // Translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f)); // Scale it down
            shader.setMat4("model", model);
            arena.Draw(shader);

            // Draw blood effects after arena to avoid shader conflicts
            boss.renderBloodEffects(debugDrawer, view, projection);
            
            // Restore main shader after debug rendering
            shader.use();

            // --- Draw Hitboxes ---
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Render in wireframe mode to see through the spheres
            // Player hitboxes
            // for (const auto& hitbox : player.attackHitboxes) {
            //     glm::vec3 color = player.isDamageActive ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
            //     debugDrawer.drawSphere(hitbox.worldPosition, hitbox.radius, color, view, projection);
            // }
            // for (const auto& hitbox : player.blockHitboxes) {
            //     glm::vec3 color = player.isBlocking ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.5f, 0.5f, 0.5f);
            //     debugDrawer.drawSphere(hitbox.worldPosition, hitbox.radius, color, view, projection);
            // }
            // for (const auto& hitbox : boss.attackHitboxes) {
            //     glm::vec3 color = boss.isDamageActive ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
            //     debugDrawer.drawSphere(hitbox.worldPosition, hitbox.radius, color, view, projection);
            // }
            // for (const auto& hitbox : boss.blockHitboxes) {
            //     glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);
            //     debugDrawer.drawSphere(hitbox.worldPosition, hitbox.radius, color, view, projection);
            // }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Switch back to fill mode

            glm::vec3 cameraOffset(0.0f, 1.5f, 3.0f); // Adjusted height and distance
            float panSpeed = 1.0f; // Smooth horizontal panning speed
            float verticalPanSpeed = 1.0f; // Smooth vertical panning speed
            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                camera.shake(0.2f, 0.1f);
            }
            camera.updateShake(deltaTime);
            camera.FollowPlayerWithOffset(player.position, cameraOffset, panSpeed, verticalPanSpeed, deltaTime);
            //camera.FollowPlayerSmoothSpring(player.position, cameraOffset, deltaTime);

            camera.updateShake(deltaTime);

            // Render UI elements (health and stamina bars)
            glDisable(GL_DEPTH_TEST); // Disable depth testing for UI rendering
            ui.renderHealthBar(player.health, 100.0f);
            ui.renderStaminaBar(player.stamina, 100.0f);
            if (!boss.isDead()) {
                ui.renderBossHealthBar(boss.health, boss.maxHealth, "Mutant Boss");
            }
            glEnable(GL_DEPTH_TEST); // Re-enable depth testing
        }
        else if (State == GAME_MENU)
        {
            glDisable(GL_DEPTH_TEST);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Switch to menu music
            if (soundEngine.getCurrentMusic() != "menu") {
                soundEngine.switchToMusic("menu");
            }

            menu.draw();
        }
        else if (State == GAME_WIN)
        {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Switch to menu music
            if (soundEngine.getCurrentMusic() != "menu") {
                soundEngine.switchToMusic("menu");
            }

            menu.drawWinMenu();
            glDisable(GL_BLEND);
        }
        else if (State == GAME_LOSE)
        {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Switch to menu music
            if (soundEngine.getCurrentMusic() != "menu") {
                soundEngine.switchToMusic("menu");
            }

            menu.drawLoseMenu();
            glDisable(GL_BLEND);
        }


        // SFML Sound Test - Press T to play test sound
        if (State == GAME_ACTIVE && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!soundEngine.isSoundPlaying("test")) {
                soundEngine.playSound("test");
                std::cout << "Playing test sound!" << std::endl;
            }
        }
        
        // Debug controls for boss testing
        if (State == GAME_ACTIVE) {
            static bool bKeyPressed = false;
            static bool nKeyPressed = false;
            
            // Press B to damage boss (for testing)
            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bKeyPressed) {
                boss.takeDamage(50);
                std::cout << "Boss health: " << boss.health << "/" << boss.maxHealth << std::endl;
                bKeyPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
                bKeyPressed = false;
            }
            
            // Press N to respawn boss (for testing)
            if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nKeyPressed) {
                boss.health = boss.maxHealth;                boss.bossState = BOSS_IDLE;
                boss.position = glm::vec3(5.0f, -0.6f, 5.0f);
                std::cout << "Boss respawned!" << std::endl;
                nKeyPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
                nKeyPressed = false;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        //std::cout << "Left click: " << left_mouse_button_pressed << " Right click: " << right_mouse_button_pressed << std::endl;
        //std::cout << "Player position: (" << player.position.x << ", " << player.position.y << ", " << player.position.z << ")\n";
        //std::cout << "CharState: " << player.charState << std::endl;
        //std::cout << "Stamina: " << player.stamina << std::endl;
        //std::cout << "is blocking: " << player.isBlocking << std::endl;
        //std::cout << "blend amount: " << player.blendAmount << std::endl;
    }

    glfwTerminate();
    return 0;
}

void initGame(Player& player, Boss& boss, TrainingDummy& dummy, PlayerCamera& camera, SoundEngine& soundEngine)
{
    player = Player();
    player.setSoundEngine(&soundEngine);
    player.setCameraReference(&camera);
    
    boss = Boss();
    boss.setSoundEngine(&soundEngine);
    boss.setPlayerReference(&player);
    
    dummy = TrainingDummy();
    soundEngine.stopAllSounds();
}

void Application::resetGame()
{
    if (player && boss && dummy && camera && soundEngine) {
        initGame(*player, *boss, *dummy, *camera, *soundEngine);
    }
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
    if (State == GAME_MENU && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->menu->isStartButtonClicked(xpos, ypos))
        {
            State = GAME_STARTING;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    else if ((State == GAME_WIN || State == GAME_LOSE) && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        
        if (app->menu->isRestartButtonClicked(xpos, ypos))
        {
            // Reset game state and restart
            app->resetGame();
            State = GAME_MENU;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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