#include "PlayerCamera.h"

PlayerCamera::PlayerCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Camera(position, up, yaw, pitch), firstMouse(true), lastX(0.0f), lastY(0.0f)
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

    ProcessMouseMovement(xoffset, yoffset);
}

void PlayerCamera::ProcessScrollCallback(double yoffset)
{
    ProcessMouseScroll(yoffset);
}

void PlayerCamera::FollowPlayer(const glm::vec3& playerPosition, const glm::vec3& offset) {
    glm::vec3 desiredPosition = playerPosition - Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);
    Position = desiredPosition;
}

void PlayerCamera::FollowPlayerWithOffset(const glm::vec3& playerPosition, const glm::vec3& offset, float panSpeed, float verticalPanSpeed) {
    glm::vec3 desiredPosition = playerPosition - Front * offset.z + glm::vec3(0.0f, offset.y, 0.0f);
    Position.x += (desiredPosition.x - Position.x) * panSpeed;
    Position.z += (desiredPosition.z - Position.z) * panSpeed;
    Position.y += (desiredPosition.y - Position.y) * verticalPanSpeed;
}
