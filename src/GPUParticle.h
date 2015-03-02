#ifndef GPU_PARTICLE
#define GPU_PARTICLE

#include <string>
#include "glm\glm.hpp"

using glm::vec3;
using glm::vec4;

struct GPUParticle
{
	//Initialise all particles as dead.
	GPUParticle() : lifeTime(1), lifeSpan(0) {}

	vec3 position;
	vec3 velocity;
	float lifeTime;
	float lifeSpan;
};

class GPUParticleEmitter
{
public:
	GPUParticleEmitter();
	GPUParticleEmitter(const unsigned int a_maxParticles, const float a_lifeSpanMin, const float a_lifeSpanMax,
					   const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize,
					   const const vec4& a_startColour, const vec4& a_endColour);
	~GPUParticleEmitter();

	void Draw(const float a_time, const glm::mat4& a_cameraTransform, const glm::mat4& a_projectionView);

private:
	void SetupBuffer(const unsigned int a_index);
	void CreateUpdateProgram();
	void CreateDrawProgram();

	unsigned int LoadShader(const std::string& a_path, const unsigned int a_type);

	GPUParticle* m_particles;

	unsigned int m_maxParticles;

	vec3 m_position;

	float m_lifeSpanMin, m_lifeSpanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColour, m_endColour;

	unsigned int m_activeBuffer;
	unsigned int m_vao[2];
	unsigned int m_vbo[2];

	unsigned int m_drawProgram, m_updateProgram;

	//Update program uniform locations
	unsigned int m_timeUniformLocation, m_deltaTimeUniformLocation, m_emitterPositionUniformLocation;
	//Draw program uniform locations
	unsigned int m_projectionViewUniformLocation, m_cameraTransformUniformLocation;


	float m_lastDrawTime;
};

#endif