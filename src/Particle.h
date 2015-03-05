#ifndef PARTICLE_H
#define PARTICLE_H

#include "glm\glm.hpp"

using glm::vec3;
using glm::vec4;

struct Particle
{
	vec3 position;
	vec3 velocity;
	vec4 colour;
	float size;
	float lifeTime;
	float lifeSpan;
};

struct ParticleVertex 
{
	vec4 position;
	vec4 colour;
};

class ParticleEmitter 
{
public:
	ParticleEmitter();
	ParticleEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax, const float a_velocityMin,
					const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour);
	~ParticleEmitter();


	void Update(const float a_deltaTime, const glm::mat4& a_cameraTransform);
	void Draw();

	inline const vec3& GetPosition()
	{
		return m_position;
	}
	inline void SetPosition(const vec3& a_position)
	{
		m_position = a_position;
	}

private:
	void Emit();

	Particle* m_particles;
	unsigned int m_firstDead;
	unsigned int m_maxParticles;

	unsigned int m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;

	vec3 m_position;

	float m_emitTimer, m_emitRate;
	float m_lifeSpanMin, m_lifeSpanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColour, m_endColour;
};


#endif