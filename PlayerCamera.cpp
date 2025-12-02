#include "PlayerCamera.h"
#include <cmath>

PlayerCamera::PlayerCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Camera(position, up, yaw, pitch), firstMouse(true), lastX(0.0f), lastY(0.0f), targetYaw(yaw), targetPitch(pitch), mouseSmoothing(8.0f), isShaking(false), shakeDuration(0.0f), shakeMagnitude(0.0f), shakeTimer(0.0f)
{
    // It's better to initialize lastX and lastY with the actual screen center,
    // but we'll set them in the first mouse callback.
}

void PlayerCamera::ProcessMouseCallback(double xpos, double ypos)
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

    // Update target yaw and pitch instead of directly modifying camera
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    targetYaw += xoffset;
    targetPitch += yoffset;

    // Constrain pitch
    if (targetPitch > 15.0f)
        targetPitch = 15.0f;
    if (targetPitch < -15.0f)
        targetPitch = -15.0f;
}

void PlayerCamera::ProcessScrollCallback(double yoffset)
{
    ProcessMouseScroll(yoffset);
}

void PlayerCamera::FollowPlayer(const glm::vec3& playerPosition, const glm::vec3& offset) {
    glm::vec3 desiredPosition = playerPosition - Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);
    Position = desiredPosition;
}

void PlayerCamera::FollowPlayerWithOffset(const glm::vec3& playerPosition, const glm::vec3& offset, float panSpeed, float verticalPanSpeed, float deltaTime) {

    // Smooth mouse rotation
    float rotationSpeed = mouseSmoothing * deltaTime;
    Yaw += (targetYaw - Yaw) * rotationSpeed;
    Pitch += (targetPitch - Pitch) * rotationSpeed;
    
    // Update camera vectors after rotation smoothing
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));

    glm::vec3 desiredPosition = playerPosition - Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);
    Position.x += (desiredPosition.x - Position.x) * panSpeed;
    Position.z += (desiredPosition.z - Position.z) * panSpeed;
    Position.y += (desiredPosition.y - Position.y) * verticalPanSpeed;
}

glm::vec3 velocity(0.0f);
float stiffness = 12.0f;
float damping   = 1.8f;

void PlayerCamera::FollowPlayerSmoothSpring(
    const glm::vec3& playerPosition,
    const glm::vec3& offset,
    float deltaTime)
{
    // Smooth mouse rotation
    float rotationSpeed = mouseSmoothing * deltaTime;
    Yaw += (targetYaw - Yaw) * rotationSpeed;
    Pitch += (targetPitch - Pitch) * rotationSpeed;
    
    // Update camera vectors after rotation smoothing
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));

    glm::vec3 desiredPosition = playerPosition - Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);
    glm::vec3 displacement = desiredPosition - Position;
    glm::vec3 acceleration = displacement * stiffness - velocity * damping;

    velocity += acceleration * deltaTime;
    Position += velocity * deltaTime;
}

void PlayerCamera::shake(float duration, float magnitude) {
    isShaking = true;
    shakeDuration = duration;
    shakeMagnitude = magnitude;
    shakeTimer = duration;
}

void PlayerCamera::updateShake(float deltaTime) {
    if (isShaking) {
        if (shakeTimer > 0) {
            float randomX = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            float randomY = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            Position.x += randomX * shakeMagnitude;
            Position.y += randomY * shakeMagnitude;
            shakeTimer -= deltaTime;
        }
        else {
            isShaking = false;
        }
    }
}
