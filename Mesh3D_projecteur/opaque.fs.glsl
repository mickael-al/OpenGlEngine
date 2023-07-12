#version 330

// il n'y a plus de variable predefinie en sortie: au revoir gl_FragColor
// on peut appeler la variable comme on veut mais doit etre marquee 'out vec4'

uniform sampler2D u_Sampler; // GL_TEXTURE0

out vec4 o_FragColor;

in vec3 v_FragPosition; // dans le repere du monde
in vec3 v_Normal;
in vec2 v_TexCoords;

uniform sampler2D u_ProjSampler; // GL_TEXTURE1
in vec4 v_ShadowCoords; // 

void main(void)
{
    // attention, une normale passee par le rasterizer a ete interpolee lineaire
    // un n-lerp (normalized lerp) permet generalement de fixer ce probleme
    vec3 N = normalize(v_Normal);


    // cette fonction effectue la projection 4D->3D->2D : 
    // xyz' = xyz/w
    // xy'' = xy'/z

    vec4 projColor = textureProj(u_ProjSampler, v_ShadowCoords);
    // il y'a des effets de bord avec cette technique
    // - d'une part il n'y a pas de clamping a [0,1], meme en changeant le mode 
    // de clamping de la texture, il est preferable de le faire manuellement
    // (ou bien d'utiliser le parametre 'border' de glTexImage2D)
    // - on peut eviter les back-projection avec un test
    // (dues aux coordonnees homogenes qui se chevauchent au dela de -1/+1)
    vec3 projectorTexCoords = v_ShadowCoords.xyz/v_ShadowCoords.z;
    if (projectorTexCoords.x < 0.0 || projectorTexCoords.x > 1.0 
     || projectorTexCoords.y < 0.0 || projectorTexCoords.y > 1.0) 
    {
        projColor = vec4(1.0);
    }

    // on peut egalement tester les valeurs xyz pour eviter 
    // l'influence de la bordure qui se repeterait

    o_FragColor = projColor * texture(u_Sampler, v_TexCoords);

    // debug des normales
    //o_FragColor = vec4(v_Normal * 0.5 + 0.5, 1.0);
}