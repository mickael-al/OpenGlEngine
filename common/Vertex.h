#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Vertex
{
	vec3 position;			//  3x4 octets = 12
	vec3 normal;			// +3x4 octets = 24
	vec2 texcoords;			// +2x4 octets = 32
	//uint8_t color[4];		// +4 octets = 36 non formellement nécessaire

	static constexpr float EPSILON = 0.001f;
	static inline bool IsSame(const vec2& lhs, const vec2& rhs)
	{
		if (fabsf(lhs.x - rhs.x) < EPSILON && fabsf(lhs.y - rhs.y) < EPSILON)
			return true;
		return false;
	}
	static inline bool IsSame(const vec3& lhs, const vec3& rhs)
	{
		if (fabsf(lhs.x - rhs.x) < EPSILON && fabsf(lhs.y - rhs.y) < EPSILON && fabsf(lhs.z - rhs.z) < EPSILON)
			return true;
		return false;
	}
	inline bool IsSame(const Vertex& v) const
	{
		if (IsSame(position, v.position)
			&& IsSame(normal, v.normal)
			&& IsSame(texcoords, v.texcoords))
			return true;
		return false;
	}
};

