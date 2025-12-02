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
    void FollowPlayerWithOffset(const glm::vec3& playerPosition, const glm::vec3& offset, float panSpeed, float verticalPanSpeed, float deltaTime);
    void updateShake(float deltaTime);
    void shake(float duration, float magnitude);
    void FollowPlayerSmoothSpring(const glm::vec3& playerPosition, const glm::vec3& offset, float deltaTime);
    
    // Mouse smoothing controls
    void setMouseSmoothing(float smoothing) { mouseSmoothing = smoothing; }
    float getMouseSmoothing() const { return mouseSmoothing; }

private:
    bool firstMouse;
    float lastX;
    float lastY;

    // Mouse smoothing
    float targetYaw;
    float targetPitch;
    float mouseSmoothing;

    bool isShaking;
    float shakeDuration;
    float shakeMagnitude;
    float shakeTimer;
};

#endif // PLAYER_CAMERA_H
