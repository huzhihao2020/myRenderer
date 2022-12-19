#version 410
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 4) in ivec4 boneIds; 
layout (location = 5) in vec4 weights;

out vec2 TexCoords;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
uniform mat4 projection_view_model;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_final_bones_matrices[MAX_BONES];

void main()
{

    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1)
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(aPos,1.0f);
            break;
        }
        vec4 localPosition = u_final_bones_matrices[boneIds[i]] * vec4(aPos,1.0f);
        totalPosition += localPosition * weights[i];
    }

    TexCoords = aTexCoords;
    gl_Position = projection_view_model * totalPosition;
    // gl_Position = projection_view_model * vec4(aPos, 1.0);
    // gl_Position = totalPosition;
}
