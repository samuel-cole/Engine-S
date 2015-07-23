#include "GPUParticle.h"
#include "gl_core_4_4.h"
#include <fstream>

GPUParticleEmitter::GPUParticleEmitter() : m_particles(nullptr), m_maxParticles(0), m_position(0, 0, 0), m_drawProgram(-1), m_updateProgram(-1), m_lastDrawTime(0), 
m_deltaTimeUniformLocation(-1), m_emitterPositionUniformLocation(-1), m_emitterPosition2UniformLocation(-1), m_timeUniformLocation(-1), m_directionUniformLocation(-1), m_depthTextureUniformLocation(-1), m_textureUniformLocation(-1)
{
	m_vao[0] = 0;
	m_vao[1] = 0;
	m_vbo[0] = 0;
	m_vbo[1] = 0;
}

unsigned int GPUParticleEmitter::s_defaultTexture = -1;

GPUParticleEmitter::GPUParticleEmitter(const unsigned int a_maxParticles, const float a_lifeSpanMin, const float a_lifeSpanMax,
									   const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize,
									   const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const unsigned int a_texture) 
									   : m_deltaTimeUniformLocation(-1), m_emitterPositionUniformLocation(-1), m_emitterPosition2UniformLocation(-1), m_timeUniformLocation(-1), m_directionUniformLocation(-1), m_depthTextureUniformLocation(-1), m_textureUniformLocation(-1)

{
	m_startColour = a_startColour;
	m_endColour = a_endColour;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_lifeSpanMin = a_lifeSpanMin;
	m_lifeSpanMax = a_lifeSpanMax;
	m_maxParticles = a_maxParticles;
	m_direction = a_direction;
	m_texture = a_texture;
	if (m_direction.x == 0)
		m_direction.x += 0.000001f;
	if (m_direction.y == 0)
		m_direction.y += 0.000001f;
	if (m_direction.z == 0)
		m_direction.z += 0.000001f;
	m_directionVariation = a_directionVariance;
	m_position = vec3(0, 0, 0);

	m_particles = new GPUParticle[a_maxParticles];

	m_activeBuffer = 0;


	glGenVertexArrays(2, m_vao);
	glGenBuffers(2, m_vbo);

	SetupBuffer(0);
	SetupBuffer(1);

	CreateUpdateProgram();

	CreateDrawProgram();
}

GPUParticleEmitter::GPUParticleEmitter(const unsigned int a_maxParticles, const float a_lifeSpanMin, const float a_lifeSpanMax,
									   const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize,
									   const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const unsigned int a_texture,
									   TwBar* const a_bar, const unsigned int a_emitterID, bool a_spinny)
									   : m_deltaTimeUniformLocation(-1), m_emitterPositionUniformLocation(-1), m_emitterPosition2UniformLocation(-1), m_timeUniformLocation(-1), m_directionUniformLocation(-1), m_depthTextureUniformLocation(-1), m_textureUniformLocation(-1)

{
	m_startColour = a_startColour;
	m_endColour = a_endColour;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_lifeSpanMin = a_lifeSpanMin;
	m_lifeSpanMax = a_lifeSpanMax;
	m_maxParticles = a_maxParticles;
	m_direction = a_direction;
	m_texture = a_texture;
	m_spinny = a_spinny;
	if (m_direction.x == 0)
		m_direction.x += 0.000001f;
	if (m_direction.y == 0)
		m_direction.y += 0.000001f;
	if (m_direction.z == 0)
		m_direction.z += 0.000001f;

	//This will only work for particle emitters with an id of less than 30 or so digits, but if I have more than 100000000000000000000000000000 emitters then I think I have bigger problems.
	char name [50];
	sprintf_s(name, "Particle Direction %u\0", a_emitterID);

	TwAddVarRW(a_bar, name, TW_TYPE_DIR3F, &m_direction[0], "");
	m_directionVariation = a_directionVariance;

	m_particles = new GPUParticle[a_maxParticles];

	m_activeBuffer = 0;


	glGenVertexArrays(2, m_vao);
	glGenBuffers(2, m_vbo);

	SetupBuffer(0);
	SetupBuffer(1);

	CreateUpdateProgram();

	CreateDrawProgram();
}

GPUParticleEmitter::~GPUParticleEmitter()
{
	delete[] m_particles;

	glDeleteVertexArrays(2, m_vao);
	glDeleteBuffers(2, m_vbo);

	if (m_drawProgram != -1)
	{
		glDeleteProgram(m_drawProgram);
		m_drawProgram = -1;
	}
	if (m_updateProgram != -1)
	{
		glDeleteProgram(m_updateProgram);
		m_updateProgram = -1;
	}
}

void GPUParticleEmitter::SetupBuffer(const unsigned int a_index)
{
	//Bind the buffer
	glBindVertexArray(m_vao[a_index]);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[a_index]);

	//Enable position, velocity, lifetime and lifespan
	glEnableVertexAttribArray(0);	
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	//Tell OpenGL how the memory is formatted
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)(sizeof(vec3)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)(sizeof(vec3) * 2));
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)(sizeof(vec3) * 2 + sizeof(float)));

	//Give OpenGL the particles to load in
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * sizeof(GPUParticle), m_particles, GL_STREAM_DRAW);

	//Unbind the buffer
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GPUParticleEmitter::CreateUpdateProgram()
{
	unsigned int vs = LoadShader("../data/shaders/vertGPUParticlesUpdate.txt", GL_VERTEX_SHADER);
	
	m_updateProgram = glCreateProgram();

	glAttachShader(m_updateProgram, vs);

	//Specify the data that will be streamed back.
	const char *varyings[] = { "vPosition", "vVelocity", "vLifeTime", "vLifeSpan" };
	glTransformFeedbackVaryings(m_updateProgram, 4, varyings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(m_updateProgram);

	int success = GL_FALSE;

	glGetProgramiv(m_updateProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_updateProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];

		glGetProgramInfoLog(m_updateProgram, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(vs);

	glUseProgram(m_updateProgram);

	//Add information that won't change.
	int location = glGetUniformLocation(m_updateProgram, "LifeMin");
	glUniform1f(location, m_lifeSpanMin);
	location = glGetUniformLocation(m_updateProgram, "LifeMax");
	glUniform1f(location, m_lifeSpanMax);
	location = glGetUniformLocation(m_updateProgram, "VelocityMin");
	glUniform1f(location, m_velocityMin);
	location = glGetUniformLocation(m_updateProgram, "VelocityMax");
	glUniform1f(location, m_velocityMax);
	location = glGetUniformLocation(m_updateProgram, "DirectionVariance");
	glUniform1f(location, m_directionVariation);
	location = glGetUniformLocation(m_updateProgram, "Spinny");
	glUniform1i(location, (m_spinny ? 1 : 0));

	//Set the location of uniforms that will change.
	m_timeUniformLocation = glGetUniformLocation(m_updateProgram, "Time");
	m_deltaTimeUniformLocation = glGetUniformLocation(m_updateProgram, "DeltaTime");
	m_emitterPositionUniformLocation = glGetUniformLocation(m_updateProgram, "EmitterPosition");
	m_emitterPosition2UniformLocation = glGetUniformLocation(m_updateProgram, "OtherEmitterPosition");
	m_directionUniformLocation = glGetUniformLocation(m_updateProgram, "EmitterDirection");
}

void GPUParticleEmitter::CreateDrawProgram()
{
	unsigned int vs = LoadShader("../data/shaders/vertGPUParticles.txt", GL_VERTEX_SHADER);
	unsigned int gs = LoadShader("../data/shaders/geomGPUParticles.txt", GL_GEOMETRY_SHADER);
	unsigned int fs = LoadShader("../data/shaders/fragGPUParticles.txt", GL_FRAGMENT_SHADER);
	
	m_drawProgram = glCreateProgram();
	glAttachShader(m_drawProgram, vs);
	glAttachShader(m_drawProgram, fs);
	glAttachShader(m_drawProgram, gs);
	glLinkProgram(m_drawProgram);

	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);

	int success = GL_FALSE;

	glGetProgramiv(m_drawProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_drawProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];

		glGetProgramInfoLog(m_drawProgram, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}

	glUseProgram(m_drawProgram);
	
	//Add information that won't change.
	int location = glGetUniformLocation(m_drawProgram, "SizeStart");
	glUniform1f(location, m_startSize);
	location = glGetUniformLocation(m_drawProgram, "SizeEnd");
	glUniform1f(location, m_endSize);
	location = glGetUniformLocation(m_drawProgram, "ColourStart");
	glUniform4fv(location, 1, &m_startColour[0]);
	location = glGetUniformLocation(m_drawProgram, "ColourEnd");
	glUniform4fv(location, 1, &m_endColour[0]);

	//Set the location of uniforms that will change.
	m_cameraTransformUniformLocation = glGetUniformLocation(m_drawProgram, "CameraTransform");
	m_projectionViewUniformLocation = glGetUniformLocation(m_drawProgram, "ProjectionView");
	m_depthTextureUniformLocation = glGetUniformLocation(m_drawProgram, "DepthTexture");
	m_textureUniformLocation = glGetUniformLocation(m_drawProgram, "Texture");
}

unsigned int GPUParticleEmitter::LoadShader(const std::string& a_path, const unsigned int a_type)
{
	//It would be nice to use the CreateShader function in Renderer for this (it's identical to this one),
	//however I prefer having my particle system as loosely coupled as possible so that it can be used in later projects.

	char* source;
	std::ifstream file(a_path);
	std::string shaderLine;
	std::string buffer;

	while (std::getline(file, shaderLine))
	{
		buffer.append(shaderLine);
		buffer.append("\n");
	}

	source = new char[buffer.length() + 1];
	for (unsigned int i = 0; i < buffer.length(); ++i)
	{
		source[i] = buffer[i];
	}
	file.close();
	source[buffer.length()] = '\0';

	unsigned int shader = glCreateShader(a_type);
	glShaderSource(shader, 1, (const char**)&source, 0);
	glCompileShader(shader);

	delete[] source;

	return shader;
}

void GPUParticleEmitter::Draw(const float a_time, const glm::mat4& a_cameraTransform, const glm::mat4& a_projectionView, const unsigned int a_depthTexture)
{
	glUseProgram(m_updateProgram);

	float deltaTime = a_time - m_lastDrawTime;
	m_lastDrawTime = a_time;

	glUniform1f(m_timeUniformLocation, a_time);
	glUniform1f(m_deltaTimeUniformLocation, deltaTime);
	glUniform3fv(m_emitterPositionUniformLocation, 1, &m_position[0]);
	glUniform3fv(m_emitterPosition2UniformLocation, 1, &m_position2[0]);
	glUniform3fv(m_directionUniformLocation, 1, &m_direction[0]);

	//Don't use the rasterizer, go to the buffer instead.
	glEnable(GL_RASTERIZER_DISCARD);

	//Bind the buffer to be updated.
	glBindVertexArray(m_vao[m_activeBuffer]);

	unsigned int otherBuffer = (m_activeBuffer == 0) ? 1 : 0;

	//Transform feedback is the buffer being updated to (I think?)- here I set the otherBuffer to be the transform feedback buffer.
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vbo[otherBuffer]);
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, m_maxParticles);

	//Disable transform feedback and enable rasterization again.
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	

	glUseProgram(m_drawProgram);

	unsigned char depthUsed = 0;
	if (a_depthTexture != -1)
	{
		//Depth
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, a_depthTexture);
		glUniform1i(m_depthTextureUniformLocation, 0);
		++depthUsed;
	}
	
	glActiveTexture(GL_TEXTURE0 + (int)depthUsed);
	glBindTexture(GL_TEXTURE_2D, (m_texture == -1 ? s_defaultTexture : m_texture));
	glUniform1i(m_textureUniformLocation, (int)depthUsed);

	glUniformMatrix4fv(m_projectionViewUniformLocation, 1, false, &a_projectionView[0][0]);
	glUniformMatrix4fv(m_cameraTransformUniformLocation, 1, false, &a_cameraTransform[0][0]);

	glBindVertexArray(m_vao[otherBuffer]);
	glDrawArrays(GL_POINTS, 0, m_maxParticles);

	m_activeBuffer = otherBuffer;
}