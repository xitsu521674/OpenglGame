#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
out vec4 color;
uniform sampler2D texture1;


void main()
{
    FragColor = texture(texture1,TexCoords);
	if(FragColor.a<0.1)
		FragColor = vec4(0.0,0,0,0.0);
}  