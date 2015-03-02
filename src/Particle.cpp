#include "Particle.h"
#include "gl_core_4_4.h"

ParticleEmitter::ParticleEmitter() : m_particles(nullptr), m_firstDead(0), m_maxParticles(0), m_position(0, 0, 0), m_vao(0), m_vbo(0), m_ibo(0), m_vertexData(nullptr)
{
}

ParticleEmitter::ParticleEmitter(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifespanMin, float a_lifespanMax, float a_velocityMin,
								float a_velocityMax, float a_startSize, float a_endSize, const vec4& a_startColour, const vec4& a_endColour) :
								m_emitTimer(0), m_emitRate(1.0f / a_emitRate), m_startColour(a_startColour), m_endColour(a_endColour),
								m_startSize(a_startSize), m_endSize(a_endSize), m_velocityMin(a_velocityMin), m_velocityMax(a_velocityMax),
								m_lifespanMin(a_lifespanMin), m_lifespanMax(a_lifespanMax), m_maxParticles(a_maxParticles), m_firstDead(0)
{
	m_particles = new Particle[m_maxParticles];
	
	m_vertexData = new ParticleVertex[m_maxParticles * 4];

	unsigned int *indexData = new unsigned int[m_maxParticles * 6];
	for (unsigned int i = 0; i < m_maxParticles; ++i)
	{
		indexData[i * 6] = i * 4;
		indexData[i * 6 + 1] = i * 4 + 1;
		indexData[i * 6 + 2] = i * 4 + 2;

		indexData[i * 6 + 3] = i * 4;
		indexData[i * 6 + 4] = i * 4 + 2;
		indexData[i * 6 + 5] = i * 4 + 3;
	}

	//Make buffers
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	//Bind buffers
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	//Enable positions and colours
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Tell OpenGL what memory format the particle vertices are in
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(ParticleVertex), m_vertexData, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxParticles * 6 * sizeof(unsigned int), indexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), ((char*)0) + 16);

	//Unbind buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indexData;
}

ParticleEmitter::~ParticleEmitter()
{
	if (m_particles != nullptr)
		delete[] m_particles;

	if (m_vertexData != nullptr)
		delete[] m_vertexData;

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
}

void ParticleEmitter::Emit()
{
	//Only emit of there is a dead particle to use
	if (m_firstDead >= m_maxParticles)
		return;

	//If there is, resurrect the first dead particle
	Particle& particle = m_particles[m_firstDead++];

	particle.position = m_position;

	particle.lifetime = 0;
	particle.lifespan = (rand() / (float)RAND_MAX) * (m_lifespanMax - m_lifespanMin) + m_lifespanMin;

	particle.colour = m_startColour;
	particle.size = m_startSize;

	float velocity = (rand() / (float)RAND_MAX) * (m_velocityMax - m_velocityMin) + m_velocityMin;
	particle.velocity.x = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.y = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.z = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity = glm::normalize(particle.velocity) * velocity;
}

void ParticleEmitter::Update(float a_deltaTime, const glm::mat4& a_cameraTransform)
{
	//Spawn particles
	m_emitTimer += a_deltaTime;
	while (m_emitTimer > m_emitRate)
	{
		Emit();
		m_emitTimer -= m_emitRate;
	}

	unsigned int quad = 0;

	//Update particles and turn live particles into billboard quads
	for (unsigned int i = 0; i < m_firstDead; ++i)
	{
		Particle* particle = &m_particles[i];

		particle->lifetime += a_deltaTime;
		if (particle->lifetime > particle->lifespan)
		{
			//Swap last alive with this one
			*particle = m_particles[m_firstDead - 1];
			--m_firstDead;
		}
		else
		{
			//Update particle
			particle->position += particle->velocity * a_deltaTime;
			particle->size = glm::mix(m_startSize, m_endSize, particle->lifetime / particle->lifespan);
			particle->colour = glm::mix(m_startColour, m_endColour, particle->lifetime / particle->lifespan);

			//Make quad
			float halfSize = particle->size * 0.5f;
			m_vertexData[quad * 4].position = vec4(halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4].colour = particle->colour;
			m_vertexData[quad * 4 + 1].position = vec4(-halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4 + 1].colour = particle->colour;
			m_vertexData[quad * 4 + 2].position = vec4(-halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 2].colour = particle->colour;
			m_vertexData[quad * 4 + 3].position = vec4(halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 3].colour = particle->colour;

			//Billboard transform
			vec3 zAxis = glm::normalize(vec3(a_cameraTransform[3]) - particle->position);
			vec3 xAxis = glm::cross(vec3(a_cameraTransform[1]), zAxis);
			vec3 yAxis = glm::cross(zAxis, xAxis);
			glm::mat4 billboard(vec4(xAxis, 0), vec4(yAxis, 0), vec4(zAxis, 0), vec4(0, 0, 0, 1));

			m_vertexData[quad * 4].position = billboard * m_vertexData[quad * 4].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 1].position = billboard * m_vertexData[quad * 4 + 1].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 2].position = billboard * m_vertexData[quad * 4 + 2].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 3].position = billboard * m_vertexData[quad * 4 + 3].position + vec4(particle->position, 0);

			++quad;
		}
	}
}

void ParticleEmitter::Draw()
{
	//Sync particle vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_firstDead * 4 * sizeof(ParticleVertex), m_vertexData);

	//Draw particles
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_firstDead * 6, GL_UNSIGNED_INT, 0);
}