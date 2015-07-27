#ifndef GPU_PARTICLE
#define GPU_PARTICLE

#include <string>
#include "glm\glm.hpp"
#include "AntTweakBar.h"

using glm::vec3;
using glm::vec4;

//Struct for particles to be ran on the GPU.
struct GPUParticle
{
	//Initialise all particles as dead.
	GPUParticle() : lifeTime(1), lifeSpan(0) {}

	vec3 position;
	vec3 velocity;
	float lifeTime;
	float lifeSpan;
};

//Class for emitters that create particles ran on the GPU.
class GPUParticleEmitter
{
public:
	GPUParticleEmitter();
	//GPU Particle Emitter constructor. Setting the texture to -1 uses the default texture instead.
	GPUParticleEmitter(const unsigned int a_maxParticles, const float a_lifeSpanMin, const float a_lifeSpanMax,
					   const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize,
					   const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const unsigned int a_texture);
	//GPU Particle Emitter constructor. Setting the texture to -1 uses the default texture instead.
	GPUParticleEmitter(const unsigned int a_maxParticles, const float a_lifeSpanMin, const float a_lifeSpanMax,
					   const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize,
					   const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const unsigned int a_texture,
					   TwBar* const a_bar, const unsigned int a_emitterID, const bool a_spinny);
	~GPUParticleEmitter();

	//Draw function. 
	//The optional a_depthTexture argument is for depth testing to make sure particles don't draw over everything else in deferred rendering.
	//The optional a_lightTexture argument is for lighting particles in deferred rendering.
	void Draw(const float a_time, const glm::mat4& a_cameraTransform, const glm::mat4& a_projectionView, const unsigned int a_depthTexture = -1);

	inline const vec3& GetPosition()
	{
		return m_position;
	}
	inline void SetPosition(const vec3& a_position)
	{
		m_position = a_position;
		m_position2 = a_position;
	}
	//Function for setting the two positions used to make a cube that particles can spawn in.
	inline void SetPosition(const vec3& a_position, const vec3& a_position2)
	{
		m_position = a_position;
		m_position2 = a_position2;
	}
	inline const vec3& GetDirection()
	{
		return m_direction;
	}
	inline void SetDirection(const vec3& a_direction)
	{
		m_direction = a_direction;
		if (m_direction.x == 0)
			m_direction.x += 0.000001f;
		if (m_direction.y == 0)
			m_direction.y += 0.000001f;
		if (m_direction.z == 0)
			m_direction.z += 0.000001f;
	}

	inline static void SetDefaultTexture(const unsigned int a_texture)
	{
		s_defaultTexture = a_texture;
	}

private:
	void SetupBuffer(const unsigned int a_index);
	void CreateUpdateProgram();
	void CreateDrawProgram();

	unsigned int LoadShader(const std::string& a_path, const unsigned int a_type);

	GPUParticle* m_particles;

	unsigned int m_maxParticles;

	float m_directionVariation;

	vec3 m_position;
	//Position2 is used for creating cubes that the particle can spawn in- particles will be spawned at a random point in-between m_position and m_position2.
	vec3 m_position2;
	vec3 m_direction;

	float m_lifeSpanMin, m_lifeSpanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColour, m_endColour;

	//Whether or not the particles should spin around and move upwards.
	bool m_spinny;

	unsigned int m_activeBuffer;
	unsigned int m_vao[2];
	unsigned int m_vbo[2];

	unsigned int m_texture;
	static unsigned int s_defaultTexture;

	//Not sure if these should work this way. On the one hand, many of the variables used don't change at all for a particle system, so the current way of 'set and forget' works well, but on the other hand, it seems ridiculous to have a program per emitter.
	//Would probably be better to have these as static and change various other things, but not worth changing in the little time left- additional features more important.
	unsigned int m_drawProgram, m_updateProgram;

	//Update program uniform locations
	unsigned int m_timeUniformLocation, m_deltaTimeUniformLocation, m_emitterPositionUniformLocation, m_emitterPosition2UniformLocation, m_directionUniformLocation;
	//Draw program uniform locations
	unsigned int m_projectionViewUniformLocation, m_cameraTransformUniformLocation, m_depthTextureUniformLocation, m_textureUniformLocation;


	float m_lastDrawTime;
};

#endif