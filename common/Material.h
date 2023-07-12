#pragma once

#include "Vertex.h"

struct Material
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	float shininess;

	uint32_t ambientTexture;	// optionnelle
	uint32_t diffuseTexture;
	uint32_t specularTexture;	// optionnelle

	static Material defaultMaterial;
};

