#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 Ambient;
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float Shininess;
uniform bool noTexture;
void main()
{    
    if(noTexture){
        FragColor = vec4(Diffuse,1.0f);
    }else{
        FragColor = texture(texture_diffuse1, TexCoords);
    }
}