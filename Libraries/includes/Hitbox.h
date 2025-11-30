#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

// Represents a simple spherical hitbox attached to a bone.
struct Hitbox {
    std::string boneName; // Name of the bone to attach to
    glm::vec3 offset;     // Positional offset from the bone's origin
    float radius;         // Radius for a spherical hitbox

    // The final, world-space position of the hitbox after transformation.
    // This is updated every frame.
    glm::vec3 worldPosition; 
};
