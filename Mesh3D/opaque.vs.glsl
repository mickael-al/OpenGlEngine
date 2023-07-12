#version 330

// role du Vertex Shader:
// produire (au minimum) une position

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_TexCoords;

out vec3 v_FragPosition; // pour les calculs d'illumination
out vec2 v_TexCoords;
out vec3 v_Normal;

uniform mat4 u_WorldMatrix;

// un 'uniform block' pour nos matrices communes
uniform Matrices {
	mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
};

void main(void)
{
	v_TexCoords = a_TexCoords;

	// il ne faut pas oublier d'appliquer les transformations a la normale
	// ... sauf qu'on ne doit pas appliquer la translation (d'ou le mat3)
	// ... de plus ... si la matrice a du scale (ou pire non-uniform)
	// alors la normale peut etre perturbee : on corrige en appliquant 
	// la transposee de l'inverse de la Transform de l'objet
	v_Normal = mat3(transpose(inverse(u_WorldMatrix))) * a_Normal;
	
	vec4 position_world = u_WorldMatrix * vec4(a_Position, 1.0);

	v_FragPosition = position_world.xyz;

	gl_Position = u_ProjectionMatrix * u_ViewMatrix * position_world;
}
