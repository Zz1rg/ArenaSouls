#include "Entity.h"

BaseEntity::BaseEntity(const std::string& modelPath, Animation* initialAnimation)
    : model(modelPath),
      animator(initialAnimation),
      position(0.0f),
      rotation(0.0f),
      scale(1.0f),
      blendAmount(0.0f),
      blendRate(0.055f),
      isDamageActive(false) {}

void BaseEntity::draw(Shader& shader) {
    auto transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}

void BaseEntity::updateHitboxes(std::vector<Hitbox>& hitboxes) {
    auto finalBoneMatrices = animator.GetFinalBoneMatrices();
    auto boneInfoMap = model.GetBoneInfoMap();

    for (auto& hitbox : hitboxes) {
        if (boneInfoMap.find(hitbox.boneName) != boneInfoMap.end()) {
            glm::mat4 boneMatrix = finalBoneMatrices[boneInfoMap[hitbox.boneName].id];
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, position);
            modelMatrix = glm::scale(modelMatrix, scale);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            hitbox.worldPosition = modelMatrix * boneMatrix * glm::vec4(hitbox.offset, 1.0f);
        }
    }
}

