#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform bool useBones;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 totalPosition = vec4(pos, 1.0f);
    vec3 totalNormal = norm;

    if (useBones)
    {
        mat4 boneTransform = mat4(0.0);
        totalPosition = vec4(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;
            if(boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(pos, 1.0f);
                break;
            }
            boneTransform += finalBonesMatrices[boneIds[i]] * weights[i];
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
            totalPosition += localPosition * weights[i];
        }
        // Transform the normal by the bone transformation matrix
        // Note: This is a simplification. For non-uniform scaling, a more complex calculation is needed.
        mat3 normalMatrix = mat3(transpose(inverse(boneTransform)));
        totalNormal = normalize(normalMatrix * norm);
    }

    FragPos = vec3(model * totalPosition);
    // Transform normals to world space
    Normal = mat3(transpose(inverse(model))) * totalNormal;

    gl_Position = projection * view * model * totalPosition;
    TexCoords = tex;
}
