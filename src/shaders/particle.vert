#version 330 core
layout (location = 0) in vec3 vertex; // <vec2 position, vec2 texCoords>
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoords;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec2 offset;

void main()
{
    vec3 tmp = vertex;
    tmp.x += offset.x;
    tmp.z += offset.y;
    gl_Position = projection * view * model * vec4(tmp,1.0f);   
    TexCoords = vec2(aTexCoord.x, aTexCoord.y);

}