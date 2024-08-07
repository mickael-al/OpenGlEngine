#version 450

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec3 ViewPos;

uniform samplerCube environmentMap;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColorSpec;
layout(location = 3) out vec2 gOther;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    gPosition.rgb = ViewPos;
    gColorSpec.rgb = envColor;
    gOther.g = 1.0;
}
