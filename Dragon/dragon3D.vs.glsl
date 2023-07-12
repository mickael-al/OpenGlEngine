#version 330

//#extension GL_ARB_shading_language_420pack : enable

// role du Vertex Shader:
// produire (au minimum) une position

// layout() est un modifier sur les variables
// ici layout(location = ) permet d'affecter directement le "canal"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out vec2 v_TexCoords;
out vec3 v_Normal;

//layout(location = 0) 
uniform mat4 u_WorldMatrix;

// uniform block (OpenGL 3+) est reference "cote shader"
// avec un index qu'on appelle "block index"
uniform Matrices 
{
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
	
	gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_WorldMatrix * vec4(a_Position, 1.0);
}
