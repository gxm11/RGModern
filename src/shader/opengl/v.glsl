#version 330 core

in vec2 position;
in vec2 TexCoord;
out vec2 sTexCoord;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    sTexCoord = TexCoord;
}
