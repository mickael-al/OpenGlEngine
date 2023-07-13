#pragma once

#include "Vertex.h"

struct SubMat
{
	alignas(16) vec3 diffuseColor;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
};

struct Material
{
	SubMat submat;

	uint32_t diffuseTexture;
	uint32_t normalMap;
	uint32_t metallicMap;
	uint32_t roughnessMap;
	uint32_t aoMap;

	static Material defaultMaterial;
};


