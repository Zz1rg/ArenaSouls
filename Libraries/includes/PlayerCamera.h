#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

#include <learnopengl/camera.h>

class PlayerCamera : public Camera
{
public:
    // Inherit constructors from Camera
    PlayerCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    void ProcessMouseCallback(double xpos, double ypos);
    void ProcessScrollCallback(double yoffset);
    void FollowPlayer(const glm::vec3& playerPosition, const glm::vec3& offset);
    void FollowPlayerWithOffset(const glm::vec3& playerPosition, const glm::vec3& offset, float panSpeed, float verticalPanSpeed);

private:
    bool firstMouse;
    float lastX;
    float lastY;
};

#endif // PLAYER_CAMERA_H
