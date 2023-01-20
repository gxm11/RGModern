#version 330 core

uniform sampler2D v_tex;
in vec2 sTexCoord;
out vec4 outColor;

void main()
{
	outColor = texture(v_tex, sTexCoord);
}