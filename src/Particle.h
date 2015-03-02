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
	float lifetime;
	float lifespan;
};

struct ParticleVertex {
	vec4 position;
	vec4 colour;
};

class ParticleEmitter {
public:
	ParticleEmitter();
	ParticleEmitter(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifespanMin, float a_lifespanMax, float a_velocityMin, 
					float a_velocityMax, float a_startSize, float a_endSize, const vec4& a_startColour, const vec4& a_endColour);
	virtual ~ParticleEmitter();


	void Update(float a_deltaTime, const glm::mat4& a_cameraTransform);
	void Draw();

protected:
	void Emit();

	Particle* m_particles;
	unsigned int m_firstDead;
	unsigned int m_maxParticles;

	unsigned int m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;

	vec3 m_position;

	float m_emitTimer, m_emitRate;
	float m_lifespanMin, m_lifespanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColour, m_endColour;
};


#endif