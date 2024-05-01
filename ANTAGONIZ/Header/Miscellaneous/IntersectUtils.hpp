#ifndef __INTERSECT_UTILS__
#define __INTERSECT_UTILS__

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

struct Box 
{
	glm::vec3 min;
	glm::vec3 max;
};

class IntersectUtils
{
	static bool rayIntersectsBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const Box& box) 
	{
		float tMin = (box.min.x - rayOrigin.x) / rayDirection.x;
		float tMax = (box.max.x - rayOrigin.x) / rayDirection.x;

		if (tMin > tMax) std::swap(tMin, tMax);

		float tyMin = (box.min.y - rayOrigin.y) / rayDirection.y;
		float tyMax = (box.max.y - rayOrigin.y) / rayDirection.y;

		if (tyMin > tyMax) std::swap(tyMin, tyMax);

		if ((tMin > tyMax) || (tyMin > tMax)) return false;

		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		float tzMin = (box.min.z - rayOrigin.z) / rayDirection.z;
		float tzMax = (box.max.z - rayOrigin.z) / rayDirection.z;

		if (tzMin > tzMax) std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax)) return false;

		return true;
	}

	static glm::vec3 getRayDirection(float mouseX, float mouseY, float screenWidth, float screenHeight, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) 
	{
		float normalizedX = (2.0f * mouseX) / screenWidth - 1.0f;
		float normalizedY = 1.0f - (2.0f * mouseY) / screenHeight;

		glm::vec4 rayClip = glm::vec4(normalizedX, normalizedY, -1.0f, 1.0f); // Coordonnées du rayon dans l'espace clip
		glm::mat4 invertedProjection = glm::inverse(projectionMatrix); // Inverse de la matrice de projection
		glm::vec4 rayEye = invertedProjection * rayClip; // Coordonnées du rayon dans l'espace œil

		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f); // Zéro pour la composante Z (en direction -Z)
		glm::mat4 invertedView = glm::inverse(viewMatrix); // Inverse de la matrice de vue
		glm::vec4 rayWorld = invertedView * rayEye; // Coordonnées du rayon dans l'espace monde
		glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld)); // Direction normalisée du rayon

		return rayDirection;
	}


};

#endif //!__INTERSECT_UTILS__