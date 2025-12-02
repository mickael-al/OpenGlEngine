#version 460 core

in VS_OUT {
    vec2 uv;
    vec3 worldPos;
    vec4 color;
} fs_in;

layout(binding = 9) uniform sampler2D fragTexture;
layout(binding = 10) uniform sampler2D u_texture;

uniform vec2 u_invResolution;

out vec3 o_FragColor;

void main()
{

    vec4 col = texture(u_texture, fs_in.uv/vec2(2,2)) * fs_in.color;
    if (fs_in.uv.x > 1.0 || fs_in.uv.y > 1.0 || col.a <= 0.01) { discard; }

    vec3 dst = texture(fragTexture, gl_FragCoord.xy * u_invResolution).rgb;    
    o_FragColor = col.rgb * col.a + dst * (1.0 - col.a);
}
