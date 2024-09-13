#version 450

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec3 ViewPos;

uniform samplerCube environmentMap;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColor;
layout(location = 3) out vec3 gOther;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    gPosition = ViewPos;
    gColor = vec4(envColor,1.0);
    gNormal.a = 1.0;
}
