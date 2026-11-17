#version 130

#if __VERSION__ >= 330
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
#else
in vec2 inPosition;
in vec2 inTexCoord;
#endif

out vec2 oTexCoord;

void main()
{
    oTexCoord = inTexCoord;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}
