#include "Renderer.h"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "glm\ext.hpp"
#include "Camera.h"
#include "FBXFile.h"
#include "Particle.h"
#include "GPUParticle.h"

#include <stb_image.h>

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

using glm::vec3;
using std::string;

Renderer::Renderer(Camera* const a_camera, TwBar* const a_bar) : m_bar(a_bar), m_file(nullptr),
m_standardProgram(-1), m_particleProgram(-1), m_noNormalsProgram(-1), m_animatedProgram(-1), m_noTexturesProgram(-1), m_noSpecularsProgram(-1), m_postProcessingProgram(-1), m_shadowGenProgram(-1)
{
	//Fill the uniform locations vector with empty vcetors. 300 should be more than enough programs.
	m_uniformLocations.assign(300, std::vector<unsigned int>());

	m_lightColour = vec3(1, 1, 1);
	m_lightDir = vec3(1, 1, 1);
	m_specPow = 2.0f;

	TwAddVarRW(m_bar, "Light Colour", TW_TYPE_COLOR3F, &m_lightColour[0], "");
	TwAddVarRW(m_bar, "Light Direction", TW_TYPE_DIR3F, &m_lightDir[0], "");
	TwAddVarRW(m_bar, "Specular Power", TW_TYPE_FLOAT, &m_specPow, "");

	//m_frameBuffers.push_back(0);
	//m_frameBufferDimensions.push_back(vec4(0, 0, 1280, 720));
	//m_frameBufferColours.push_back(vec3(0.25f, 0.25f, 0.75f));

	//Set up the framebuffer that everything will be ran through- used for post processing effects.
	unsigned int texture = LoadFrameBuffer(a_camera, vec4(0, 0, 1280, 720), vec3(0.25f, 0.25f, 0.75f));

	//Make a fullscreen quad to render the post processing framebuffer stuff through.
	glm::vec2 halfTexel = 1.0f / glm::vec2(1280, 720) * 0.5f;
	Vertex vertexArray[4];
	vertexArray[0].position = vec4(-1, -1, 0, 1);
	vertexArray[0].uv = glm::vec2(halfTexel.x, halfTexel.y);
	vertexArray[1].position = vec4(-1, 1, 0, 1);
	vertexArray[1].uv = glm::vec2(halfTexel.x, 1 - halfTexel.y);
	vertexArray[2].position = vec4(1, -1, 0, 1);
	vertexArray[2].uv = glm::vec2(1 - halfTexel.x, halfTexel.y);
	vertexArray[3].position = vec4(1, 1, 0, 1);
	vertexArray[3].uv = glm::vec2(1 - halfTexel.x, 1 - halfTexel.y);
	
	unsigned int indexArray[6];
	indexArray[0] = 0;
	indexArray[1] = 3;
	indexArray[2] = 1;
	indexArray[3] = 0;
	indexArray[4] = 2;
	indexArray[5] = 3;

	LoadIntoOpenGL(vertexArray, 4, indexArray, 6, false);
	LoadTexture(texture, 0);

	m_postProcessingProgram = CreateProgram("../data/shaders/vertPostProcessing.txt", "../data/shaders/fragPostProcessing.txt");

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

unsigned int Renderer::CreateProgram(const string& a_vertPath, const string& a_fragPath)
{
	//Vertex Shader
	unsigned int vertexShader = LoadShader(a_vertPath, GL_VERTEX_SHADER);

	//Fragment Shader
	unsigned int fragmentShader = LoadShader(a_fragPath, GL_FRAGMENT_SHADER);

	int success = GL_FALSE;

	unsigned int programID = glCreateProgram();
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];

		glGetShaderInfoLog(programID, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	//The below bit could potentially be improved by adding another parameter to CreateProgram which is used to pass in an array/vector of UniformTypes, and only finding the locations of those types.
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "ProjectionView"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Bones"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Global"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightDir"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightColour"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "CameraPos"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "SpecPow"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Diffuse"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Normal"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Specular"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightMatrix"));

	return programID;
}

unsigned int Renderer::LoadShader(const string& a_path, const unsigned int a_type)
{
	char* source;
	std::ifstream file(a_path);
	string shaderLine;
	string buffer;

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

unsigned int Renderer::LoadFrameBuffer(Camera* const a_camera, const vec4& a_dimensions, const vec3& a_backgroundColour)
{
	//Create frame buffer
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Create texture
	unsigned int FBOTexture;
	glGenTextures(1, &FBOTexture);
	glBindTexture(GL_TEXTURE_2D, FBOTexture);

	//Specify texture format
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, (GLsizei)a_dimensions.z, (GLsizei)a_dimensions.w);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Attach to the Frame Buffer as first colour attachment.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOTexture, 0);

	//Create depth buffer.
	unsigned int fboDepth;
	glGenRenderbuffers(1, &fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (GLsizei)a_dimensions.z, (GLsizei)a_dimensions.w);

	//Attach the depth buffer to the FBO.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboDepth);

	//Tell OpenGL how many colour attachments we have assigned and what they are.
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Error: Framebuffer creation failed" << std::endl;

	//Unbind framebuffer so that we can render back to the back buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_frameBuffers.push_back(FBO);
	m_frameBufferDimensions.push_back(a_dimensions);
	m_frameBufferColours.push_back(a_backgroundColour);
	
	m_cameras.push_back(a_camera);

	return FBOTexture;
}

void Renderer::LoadShadowMap()
{
	if (m_shadowGenProgram == -1)
		m_shadowGenProgram = CreateProgram("../data/shaders/vertShadowPath", "../data/shaders/fragShadowPath");

	//Generate framebuffer to store shadow map in.
	glGenFramebuffers(1, &m_shadowMap);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMap);

	unsigned int depth;
	glGenTextures(1, &depth);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Capture depth- not colour.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0);

	glDrawBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Error: Shadow map framebuffer was not created correctly." << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_lightProjection = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
}

void Renderer::LoadTexture(const string& a_filePath, const bool a_channels, unsigned int a_index)
{
	if (m_noNormalsProgram == -1)
	{
		m_noNormalsProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNorm.txt");
	}

	while (a_index >= m_textures.size())
	{
		m_textures.push_back(-1);
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_textures[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_textures[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (a_channels) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (a_channels) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::LoadTexture(const unsigned int a_textureIndex, const unsigned int a_index)
{
	if (m_noNormalsProgram == -1)
	{
		m_noNormalsProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNorm.txt");
	}

	while (a_index >= m_textures.size())
	{
		m_textures.push_back(-1);
	}

	m_textures[a_index] = a_textureIndex;
}

void Renderer::LoadNormalMap(const string& a_filePath, const bool a_channels, unsigned int a_index)
{
	//Check to see if a program that can handle normal maps has been created, and make one if it hasn't.
	if (m_noSpecularsProgram == -1)
	{
		m_noSpecularsProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoSpec.txt");
	}

	while (a_index >= m_normals.size())
	{
		m_normals.push_back(-1);
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_normals[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_normals[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (a_channels) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (a_channels) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::LoadSpecularMap(const string& a_filePath, const bool a_channels, unsigned int a_index)
{
	//Check to see if a program that can handle normal maps has been created, and make one if it hasn't.
	if (m_standardProgram == -1)
	{
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");
	}

	while (a_index >= m_speculars.size())
	{
		m_speculars.push_back(-1);
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_speculars[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_speculars[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (a_channels) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (a_channels) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

unsigned int Renderer::GenerateGrid(const unsigned int a_rows, const unsigned int a_columns, const glm::vec3& a_offset)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoTex.txt");
	}

	unsigned int rows = a_rows + 1;
	unsigned int columns = a_rows + 1;

	Vertex* aoVertices = new Vertex[ rows * columns];
	for (unsigned int r = 0; r < rows; ++r)
	{
		for (unsigned int c = 0; c < columns; ++c)
		{
			aoVertices[ r * columns + c].position = vec4((float)c + a_offset.x, a_offset.y, (float)r + a_offset.z, 1);
	
			//Creating an arbitrary colour.
			//vec3 colour(sinf( (c / (float)(columns - 1)) * (r / (float)(rows - 1))));
			vec3 colour(1,1,1);
	
			aoVertices[r * columns + c].colour = vec4(colour, 1);
			aoVertices[r * columns + c].normal = glm::vec4(0, 1, 0, 1);
			aoVertices[r * columns + c].tangent = glm::vec4(1, 0, 0, 1);
			aoVertices[r * columns + c].uv = glm::vec2((float)r / (rows / rows), (float)c / (columns / columns));
		}
	}
	
	unsigned int* auiIndices = new unsigned int[(rows - 1) * (columns - 1) * 6];

	unsigned int index = 0;
	for (unsigned int r = 0; r < (rows - 1); ++r)
	{
		for (unsigned int c = 0; c < (columns - 1); ++c)
		{
			//Triangle 1
			auiIndices[index++] = r * columns + c;
			auiIndices[index++] = (r + 1) * columns + c;
			auiIndices[index++] = (r + 1) * columns + (c + 1);

			//Triangle 2
			auiIndices[index++] = r * columns + c;
			auiIndices[index++] = (r + 1) * columns + (c + 1);
			auiIndices[index++] = r * columns + (c + 1); 
		}
	}

	LoadIntoOpenGL(aoVertices, rows * columns, auiIndices, (rows - 1) * (columns - 1) * 6, false);

	delete[] aoVertices;
	delete[] auiIndices;

	return m_numOfIndices.size() - 1;
}

unsigned int Renderer::CreateEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax,
									 const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		GPUParticleEmitter* emitter = new GPUParticleEmitter(a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour);

		std::vector<GPUParticleEmitter*>::iterator i = m_gpuEmitters.begin();
		while (i != m_gpuEmitters.end())
		{
			if (*i == nullptr)
			{
				*i = emitter;
				return i - m_gpuEmitters.begin();
			}
			++i;
		}

		m_gpuEmitters.push_back(emitter);
		return m_gpuEmitters.size() - 1;
	}
	else
	{ 
		if (m_particleProgram == -1)
		{
			m_particleProgram = CreateProgram("../data/shaders/vertParticles.txt", "../data/shaders/fragParticles.txt");
		}

		ParticleEmitter* emitter = new ParticleEmitter(a_maxParticles, a_emitRate, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour);

		std::vector<ParticleEmitter*>::iterator i = m_emitters.begin();
		while (i != m_emitters.end())
		{
			if (*i == nullptr)
			{
				*i = emitter;
				return i - m_emitters.begin();
			}
			++i;
		}

		m_emitters.push_back(emitter);
		return m_emitters.size() - 1;
	}
}

unsigned int Renderer::CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax,
	const float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		GPUParticleEmitter* emitter = new GPUParticleEmitter(a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour);

		std::vector<GPUParticleEmitter*>::iterator i = m_gpuEmitters.begin();
		while (i != m_gpuEmitters.end())
		{
			if (*i == nullptr)
			{
				*i = emitter;
				return i - m_gpuEmitters.begin();
			}
			++i;
		}

		m_gpuEmitters.push_back(emitter);
		return m_gpuEmitters.size() - 1;
	}
	else
	{
		std::cout << "Error: incorrect CreateEmitter overload called. For CPU-based particles, use the CreateEmitter function which specifies emit rate.";
		return -1;
	}
}

void Renderer::SetEmitterPosition(const unsigned int a_index, const bool a_gpuBased, const vec3& a_position)
{
	if (a_gpuBased)
	{
		if (m_gpuEmitters[a_index] != nullptr)
			m_gpuEmitters[a_index]->SetPosition(a_position);
		
		else
			std::cout << "Error: invalid emitter index!" << std::endl;
	}
	else
	{
		if (m_emitters[a_index] != nullptr)
			m_emitters[a_index]->SetPosition(a_position);

		else
			std::cout << "Error: invalid emitter index!" << std::endl;
	}
}

const vec3& Renderer::GetEmitterPosition(const unsigned int a_index, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		if (m_gpuEmitters[a_index] != nullptr)
			return m_gpuEmitters[a_index]->GetPosition();
		else
		{
			std::cout << "Error: invalid emitter index!" << std::endl;
			return *(vec3*)nullptr;
		}
	}
	else
	{
		if (m_emitters[a_index] != nullptr)
			return m_emitters[a_index]->GetPosition();
		else
		{
			std::cout << "Error: invalid emitter index!" << std::endl;
			return *(vec3*)nullptr;
		}
	}
}

void Renderer::DestroyEmitter(const unsigned int a_emitterIndex, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		if (m_gpuEmitters[a_emitterIndex] != nullptr)
		{
			delete m_gpuEmitters[a_emitterIndex];
			m_gpuEmitters[a_emitterIndex] = nullptr;
		}
	}
	else
	{
		if (m_emitters[a_emitterIndex] != nullptr)
		{
			delete m_emitters[a_emitterIndex];
			m_emitters[a_emitterIndex] = nullptr;
		}
	}
}

unsigned int Renderer::LoadFBX(const string& a_filePath)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoTex.txt");
	}

	m_file = new FBXFile();

	m_file->load(a_filePath.c_str());
	for (unsigned int j = 0; j < m_file->getMeshCount(); ++j)
	{
		FBXMeshNode* mesh = m_file->getMeshByIndex(j);
		m_file->initialiseOpenGLTextures();

		Vertex* vertices = new Vertex[mesh->m_vertices.size()];
		for (unsigned int i = 0; i < mesh->m_vertices.size(); ++i)
		{
			vertices[i].position = mesh->m_vertices[i].position;
			vertices[i].normal = mesh->m_vertices[i].normal;
			vertices[i].uv = mesh->m_vertices[i].texCoord1;
			vertices[i].colour = mesh->m_vertices[i].colour;
			vertices[i].tangent = mesh->m_vertices[i].tangent;
			vertices[i].weights = mesh->m_vertices[i].weights;
			vertices[i].indices = mesh->m_vertices[i].indices;
		}

		unsigned int* indices = new unsigned int[mesh->m_indices.size()];
		for (unsigned int i = 0; i < mesh->m_indices.size(); ++i)
		{
			indices[i] = mesh->m_indices[i];
		}

		LoadIntoOpenGL(vertices, mesh->m_vertices.size(), indices, mesh->m_indices.size(), m_file->getSkeletonCount() > 0);

		delete[] vertices;
		delete[] indices;
	}


	if (m_file->getSkeletonCount() > 0)
	{
		if (m_animatedProgram == -1)
			m_animatedProgram = CreateProgram("../data/shaders/vertAnim.txt", "../data/shaders/frag.txt");

		FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);
		FBXAnimation* animation = m_file->getAnimationByIndex(0);

		skeleton->updateBones();

		skeleton->evaluate(animation, 0);

		for (unsigned int i = 0; i < skeleton->m_boneCount; ++i)
		{
			skeleton->m_nodes[i]->updateGlobalTransform();
		}
	}

	return m_numOfIndices.size() - 1;
}

void Renderer::LoadFBX(const string& a_filePath, const std::vector<string>* const a_texturePaths, const std::vector<string>* const a_normalMapPaths, const std::vector<string>* const a_specularMapPaths,
					   const std::vector<bool>* const a_texChannels, const std::vector<bool>* const a_normChannels, const std::vector<bool>* const a_specularChannels)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoTex.txt");
	}

	m_file = new FBXFile();

	m_file->load(a_filePath.c_str());
	for (unsigned int j = 0; j < m_file->getMeshCount(); ++j)
	{
		FBXMeshNode* mesh = m_file->getMeshByIndex(j);
		m_file->initialiseOpenGLTextures();

		Vertex* vertices = new Vertex[mesh->m_vertices.size()];
		for (unsigned int i = 0; i < mesh->m_vertices.size(); ++i)
		{
			vertices[i].position = mesh->m_vertices[i].position;
			vertices[i].normal = mesh->m_vertices[i].normal;
			vertices[i].uv = mesh->m_vertices[i].texCoord1;
			vertices[i].colour = mesh->m_vertices[i].colour;
			vertices[i].tangent = mesh->m_vertices[i].tangent;
			vertices[i].weights = mesh->m_vertices[i].weights;
			vertices[i].indices = mesh->m_vertices[i].indices;
		}

		unsigned int* indices = new unsigned int[mesh->m_indices.size()];
		for (unsigned int i = 0; i < mesh->m_indices.size(); ++i)
		{
			indices[i] = mesh->m_indices[i];
		}		

		LoadIntoOpenGL(vertices, mesh->m_vertices.size(), indices, mesh->m_indices.size(), m_file->getSkeletonCount() > 0);

		delete[] vertices;
		delete[] indices;

		if (j < a_texturePaths->size())
		{
			LoadTexture((*a_texturePaths)[j], (*a_texChannels)[j], m_numOfIndices.size() - 1);
		}
		if (j < a_normalMapPaths->size())
		{
			LoadNormalMap((*a_normalMapPaths)[j], (*a_normChannels)[j], m_numOfIndices.size() - 1);
		}
		if (j < a_specularMapPaths->size())
		{
			LoadSpecularMap((*a_specularMapPaths)[j], (*a_specularChannels)[j], m_numOfIndices.size() - 1);
		}
	}


	if (m_file->getSkeletonCount() > 0)
	{
		if (m_animatedProgram == -1)
			m_animatedProgram = CreateProgram("../data/shaders/vertAnim.txt", "../data/shaders/frag.txt");

		for (unsigned int i = 0; i < m_file->getSkeletonCount(); ++i)
		{
			FBXSkeleton* skeleton = m_file->getSkeletonByIndex(i);
			FBXAnimation* animation = m_file->getAnimationByIndex(i);
			
			skeleton->updateBones();

			skeleton->evaluate(animation, 0);

			for (unsigned int j = 0; j < skeleton->m_boneCount; ++j)
			{
				skeleton->m_nodes[j]->updateGlobalTransform();
			}
		}
	}
}

unsigned int Renderer::LoadOBJ(const string& a_filePath)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoTex.txt");
	}

	//Load file
	std::ifstream file(a_filePath);
	string currentLine;
	
	//Create vectors to push things into
	std::vector<vec3> position;
	std::vector<vec3> normal;
	std::vector<glm::vec2> uv;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	unsigned int testCounter = 0;

	//Here is where I look at each line and read its values into the appropriate vector.
	//NOTE: / and // symbols within the file are not currently accounted for. (http://en.wikipedia.org/wiki/Wavefront_.obj_file)
	/*
	v = position
	vn = normals
	vt = textureCoords
	f = indices/uv/normals
	*/

	//I need to flip UVs.

	while (std::getline(file, currentLine))
	{
		++testCounter;
		if (currentLine[0] == 'v')
		{
			if (currentLine[1] == 't')			//UV
			{
				unsigned int firstSpace = currentLine.find(' ', 3);
		
				string firstNum = currentLine.substr(2, firstSpace - 1);
				string secondNum = currentLine.substr(firstSpace + 1, string::npos);
		
				uv.push_back(glm::vec2(std::stof(firstNum), 1 - std::stof(secondNum)));
			}
			else  if (currentLine[1] == 'n')	//Normal
			{
				unsigned int firstSpace = currentLine.find(' ', 3);
				unsigned int secondSpace = currentLine.find(' ', firstSpace + 1);
		
				string firstNum = currentLine.substr(2, firstSpace - 1);
				string secondNum = currentLine.substr(firstSpace + 1, secondSpace - firstSpace);
				string thirdNum = currentLine.substr(secondSpace + 1, string::npos);
		
				normal.push_back(vec3(std::stof(firstNum), std::stof(secondNum), std::stof(thirdNum)));
			}
			else if (currentLine[1] == ' ')		//Position
			{
				unsigned int firstSpace = currentLine.find(' ', 2);
				unsigned int secondSpace = currentLine.find(' ', firstSpace + 1);
		
				string firstNum = currentLine.substr(2, firstSpace - 1);
				string secondNum = currentLine.substr(firstSpace + 1, secondSpace - firstSpace);
				string thirdNum = currentLine.substr(secondSpace + 1, string::npos);
		
				position.push_back(vec3(std::stof(firstNum), std::stof(secondNum), std::stof(thirdNum)));
			}
			else
			{
				std::cout << "Invalid line in OBJ file." << std::endl;
			}
		}
		else if (currentLine[0] == 'f')			//Index
		{
			SplitIndex(currentLine, &vertices, &indices, &position, &normal, &uv);
		}
		else
		{
			std::cout << "Invalid line in OBJ file." << std::endl;
		}
	}

	file.close();
	
	Vertex* aoVertices = new Vertex[vertices.size()];
	std::vector<Vertex>::iterator iter = vertices.begin();
	while (iter != vertices.end())
	{
		//Look at the unsigned int part of the vertices.
		//At that index, give a value of the vertex key.
		aoVertices[iter - vertices.begin()] = (*iter);
		++iter;
	}

	
	unsigned int* auiIndices = new unsigned int[indices.size()];
	for (unsigned int i = 0; i < indices.size(); ++i)
	{
		auiIndices[i] = indices[i];
	}

	LoadIntoOpenGL(aoVertices, vertices.size(), auiIndices, indices.size(), false);

	delete[] aoVertices;
	delete[] auiIndices;

	return m_numOfIndices.size() - 1;
}

void Renderer::Draw()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (m_shadowGenProgram != -1)
	{
		//Render to the shadow map.
		glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMap);
		glViewport(0, 0, 1024, 1024);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_shadowGenProgram);
		mat4 lightProjView = m_lightProjection * glm::lookAt(m_lightDir, vec3(0, 0, 0), vec3(0, 1, 0));
		glUniformMatrix4fv((m_uniformLocations[m_shadowGenProgram])[LIGHT_PROJVIEW], 1, GL_FALSE, &(lightProjView[0][0]));

		for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
		{
			glBindVertexArray(m_VAO[i]);
			glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
		}
	}

	//Render everything to any framebuffers.
	for (int j = m_frameBuffers.size() - 1; j >= 0; --j)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffers[j]);
		glViewport((GLint)m_frameBufferDimensions[j].x, (GLint)m_frameBufferDimensions[j].y, (GLint)m_frameBufferDimensions[j].z, (GLint)m_frameBufferDimensions[j].w);
		glClearColor(m_frameBufferColours[j].x, m_frameBufferColours[j].y, m_frameBufferColours[j].z, 1);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawModels(j);
	}

	//Do stuff to render the framebuffer used for post processing.

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1280, 720);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_postProcessingProgram);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textures[0]);

	glBindVertexArray(m_VAO[0]);
	glDrawElements(GL_TRIANGLES, m_numOfIndices[0], GL_UNSIGNED_INT, nullptr);

}

void Renderer::DrawModels(unsigned int j)
{
	if (m_noTexturesProgram != -1)
	{
		//I use noTexturesCheck to see if there are any models with no texture. 
		//I use an int instead of a bool so that I can start at the index of the first found object without either when I start drawing.
		unsigned int noTexturesCheck = -1;
		for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
		{
			if (m_textures[i] == -1 && m_normals[i] == -1 && m_speculars[i] == -1)
			{
				noTexturesCheck = i;
				break;
			}
		}

		if (noTexturesCheck != -1)
		{
			glUseProgram(m_noTexturesProgram);

			glUniformMatrix4fv((m_uniformLocations[m_noTexturesProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

			glUniform3f((m_uniformLocations[m_noTexturesProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
			glUniform3f((m_uniformLocations[m_noTexturesProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
			glUniform3f((m_uniformLocations[m_noTexturesProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
			glUniform1f((m_uniformLocations[m_noTexturesProgram])[SPEC_POW], m_specPow);

			for (unsigned int i = noTexturesCheck; i < m_numOfIndices.size(); ++i)
			{
				if (m_textures[i] != -1 || m_normals[i] != -1 || m_speculars[i] != -1)
					continue;

				//Draw stuff
				glBindVertexArray(m_VAO[i]);
				glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
			}
		}


		//No normals program can only have a value if a no texture program has already been created, hence this is nested.
		if (m_noNormalsProgram != -1)
		{
			//I use noTexturesCheck to see if there are any models with no normal map. 
			//I use an int instead of a bool so that I can start at the index of the first found object without one when I start drawing.
			unsigned int noNormalsCheck = -1;
			for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
			{
				if (m_textures[i] != -1 && m_normals[i] == -1 && m_speculars[i] == -1)
				{
					noNormalsCheck = i;
					break;
				}
			}

			if (noNormalsCheck != -1)
			{
				glUseProgram(m_noNormalsProgram);

				glUniformMatrix4fv((m_uniformLocations[m_noNormalsProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

				glUniform3f((m_uniformLocations[m_noNormalsProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
				glUniform3f((m_uniformLocations[m_noNormalsProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
				glUniform3f((m_uniformLocations[m_noNormalsProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
				glUniform1f((m_uniformLocations[m_noNormalsProgram])[SPEC_POW], m_specPow);

				for (unsigned int i = noNormalsCheck; i < m_numOfIndices.size(); ++i)
				{
					if (m_textures[i] == -1 || m_normals[i] != -1 || m_speculars[i] != -1)
						continue;

					// Set Texture/Diffuse Slot
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, m_textures[i]);
					glUniform1i((m_uniformLocations[m_noNormalsProgram])[DIFFUSE], 0);

					//Draw stuff
					glBindVertexArray(m_VAO[i]);
					glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
				}
			}


			//Standard program can only have a value if a no normals program has already been created, hence this is nested.
			if (m_noSpecularsProgram != -1)
			{
				//I use noSpecularCheck to see if there are any models with no specular. 
				//I use an int instead of a bool so that I can start at the index of the first found object without one when I start drawing.
				unsigned int noSpecularCheck = -1;
				for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
				{
					if (m_textures[i] != -1 && m_normals[i] != -1 && m_speculars[i] == -1)
					{
						noSpecularCheck = i;
						break;
					}
				}

				if (noSpecularCheck != -1)
				{
					glUseProgram(m_noSpecularsProgram);

					glUniformMatrix4fv((m_uniformLocations[m_noSpecularsProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

					glUniform3f((m_uniformLocations[m_noSpecularsProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
					glUniform3f((m_uniformLocations[m_noSpecularsProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
					glUniform3f((m_uniformLocations[m_noSpecularsProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
					glUniform1f((m_uniformLocations[m_noSpecularsProgram])[SPEC_POW], m_specPow);

					for (unsigned int i = noSpecularCheck; i < m_numOfIndices.size(); ++i)
					{
						if (m_textures[i] == -1 || m_normals[i] == -1 || m_speculars[i] != -1)
							continue;

						// Set Texture/Diffuse Slot
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, m_textures[i]);
						glUniform1i((m_uniformLocations[m_noSpecularsProgram])[DIFFUSE], 0);

						// Set Normal Slot
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, m_normals[i]);
						glUniform1i((m_uniformLocations[m_noSpecularsProgram])[NORMAL], 1);

						//Draw stuff
						glBindVertexArray(m_VAO[i]);
						glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
					}
				}


				//Standard program can only have a value if a no normals program has already been created, hence this is nested.
				if (m_standardProgram != -1)
				{
					//I use notAnimatedCheck to see if there are any models that aren't animated. 
					//I use an int instead of a bool so that I can start at the index of the first found object that isn't animated when I start drawing.
					unsigned int notAnimatedCheck = -1;
					for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
					{
						if (m_textures[i] != -1 && m_normals[i] != -1 && m_animated[i] == false)
						{
							notAnimatedCheck = i;
							break;
						}
					}

					if (notAnimatedCheck != -1)
					{
						glUseProgram(m_standardProgram);

						glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

						glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
						glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
						glUniform3f((m_uniformLocations[m_standardProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
						glUniform1f((m_uniformLocations[m_standardProgram])[SPEC_POW], m_specPow);

						for (unsigned int i = notAnimatedCheck; i < m_numOfIndices.size(); ++i)
						{
							if (m_textures[i] == -1 || m_normals[i] == -1 || m_speculars[i] == 1 || m_animated[i])
								continue;

							// Set Texture/Diffuse Slot
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, m_textures[i]);
							glUniform1i((m_uniformLocations[m_standardProgram])[DIFFUSE], 0);

							// Set Normal Slot
							glActiveTexture(GL_TEXTURE1);
							glBindTexture(GL_TEXTURE_2D, m_normals[i]);
							glUniform1i((m_uniformLocations[m_standardProgram])[NORMAL], 1);

							// Set Specular Slot
							glActiveTexture(GL_TEXTURE1);
							glBindTexture(GL_TEXTURE_2D, m_speculars[i]);
							glUniform1i((m_uniformLocations[m_standardProgram])[SPECULAR], 1);

							//Draw stuff
							glBindVertexArray(m_VAO[i]);
							glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
						}
					}

					//Animated program can only have a value if a standard program has already been created, hence this is nested.
					if (m_animatedProgram != -1)
					{
						FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);

						glUseProgram(m_animatedProgram);

						glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));
						glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[BONES], skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);
						glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[GLOBAL], 1, GL_FALSE, &(m_file->getMeshByIndex(0)->m_globalTransform[0][0]));

						glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
						glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
						glUniform3f((m_uniformLocations[m_animatedProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
						glUniform1f((m_uniformLocations[m_animatedProgram])[SPEC_POW], m_specPow);

						for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
						{
							if (m_animated[i] == false)
								continue;

							//At the moment, only textured models with normal maps and specular maps are supported for animation, so continue if this isn't the case.
							if (m_textures[i] == -1 || m_normals[i] == -1 || m_speculars[i] == -1 || !m_animated[i])
								continue;

							// Set Texture/Diffuse Slot
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, m_textures[i]);
							glUniform1i((m_uniformLocations[m_animatedProgram])[DIFFUSE], 0);

							// Set Normal Slot
							glActiveTexture(GL_TEXTURE1);
							glBindTexture(GL_TEXTURE_2D, m_normals[i]);
							glUniform1i((m_uniformLocations[m_animatedProgram])[NORMAL], 1);

							//Set Specular Slot
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, m_speculars[i]);
							glUniform1i((m_uniformLocations[m_animatedProgram])[SPECULAR], 1);

							//Draw stuff
							glBindVertexArray(m_VAO[i]);
							glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
						}
					}
				}
			}
		}
	}

	//Particles are seperate from the huge mess of nested 'if' statements above because they are not dependant on any types of models being created.
	if (m_particleProgram != -1)
	{
		glUseProgram(m_particleProgram);

		glUniformMatrix4fv((m_uniformLocations[m_particleProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

		for (unsigned int i = 0; i < m_emitters.size(); ++i)
		{
			if (m_emitters[i] != nullptr)
				m_emitters[i]->Draw();
		}
	}

	for (unsigned int i = 0; i < m_gpuEmitters.size(); ++i)
	{
		if (m_gpuEmitters[i] != nullptr)
		{
			m_gpuEmitters[i]->Draw((float)glfwGetTime(), m_cameras[j]->GetWorldTransform(), m_cameras[j]->GetProjectionView());
		}
	}
}

void Renderer::UpdateAnimation(const float a_time)
{
	if (m_file != nullptr && m_file->getSkeletonCount() > 0)
	{
		FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);
		skeleton->updateBones();

		skeleton->evaluate(m_file->getAnimationByIndex(0), a_time);

		for (unsigned int i = 0; i < skeleton->m_boneCount; ++i)
		{
			skeleton->m_nodes[i]->updateGlobalTransform();
		}
	}
}

void Renderer::UpdateEmitters(const float a_deltaTime)
{
	for (unsigned int i = 0; i < m_emitters.size(); ++i)
	{
		if (m_emitters[i] != nullptr)
			m_emitters[i]->Update(a_deltaTime, m_cameras[0]->GetWorldTransform());
	}
}

void Renderer::SplitIndex(const string& a_string, std::vector<Vertex>* const a_vertices, std::vector<unsigned int>* const a_indices, std::vector<vec3>* const a_position, std::vector<vec3>* a_normal, std::vector<glm::vec2>* a_uv)
{
	unsigned int firstSpace = a_string.find(' ', 2);
	unsigned int secondSpace = a_string.find(' ', firstSpace + 1);

	string firstNum = a_string.substr(2, firstSpace - 1);
	string secondNum = a_string.substr(firstSpace + 1, secondSpace - firstSpace);
	string thirdNum = a_string.substr(secondSpace + 1, string::npos);

	unsigned int firstNumFirstSlash = firstNum.find('/', 0);
	if (firstNumFirstSlash == string::npos)	//No slashes
	{
		GenerateVertFromIndices(firstNum,  "", "", a_position, a_normal, a_uv, a_vertices, a_indices);
		GenerateVertFromIndices(secondNum, "", "", a_position, a_normal, a_uv, a_vertices, a_indices);
		GenerateVertFromIndices(thirdNum,  "", "", a_position, a_normal, a_uv, a_vertices, a_indices);
	}
	else
	{
		unsigned int secondNumFirstSlash = secondNum.find('/', 0);
		unsigned int thirdNumFirstSlash = thirdNum.find('/', 0);

		string firstNumIndex = firstNum.substr(0, firstNumFirstSlash);
		string secondNumIndex = secondNum.substr(0, secondNumFirstSlash);
		string thirdNumIndex = thirdNum.substr(0, thirdNumFirstSlash);

		unsigned int firstNumSecondSlash = firstNum.find('/', firstNumFirstSlash + 1);
		
		if (firstNumSecondSlash == string::npos)
		{
			//One single slash

			//Get info out of string...
			string firstNumUV = firstNum.substr(firstNumFirstSlash + 1, string::npos);
			string secondNumUV = secondNum.substr(secondNumFirstSlash + 1, string::npos);
			string thirdNumUV = thirdNum.substr(thirdNumFirstSlash + 1, string::npos);

			//...and into vertices.
			GenerateVertFromIndices(firstNumIndex,  "", firstNumUV,  a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(secondNumIndex, "", secondNumUV, a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(thirdNumIndex,  "", thirdNumUV,  a_position, a_normal, a_uv, a_vertices, a_indices);
		}
		else if (firstNumSecondSlash == firstNumFirstSlash + 1)
		{
			//Get info out of string...
			//First num stuff
			string firstNumNormal = firstNum.substr(firstNumSecondSlash + 1, string::npos);
			//Second num stuff
			string secondNumNormal = secondNum.substr(secondNumFirstSlash + 2, string::npos);
			//Third num stuff
			string thirdNumNormal = thirdNum.substr(thirdNumFirstSlash + 2, string::npos);

			//...and into vertices.
			GenerateVertFromIndices(firstNumIndex,  firstNumNormal,  "", a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(secondNumIndex, secondNumNormal, "", a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(thirdNumIndex,  thirdNumNormal,  "", a_position, a_normal, a_uv, a_vertices, a_indices);
		}
		else
		{
			//Two single slash

			//Get info out of string...
			//First num stuff
			string firstNumUV = firstNum.substr(firstNumFirstSlash + 1, firstNumSecondSlash - firstNumFirstSlash - 1);
			string firstNumNormal = firstNum.substr(firstNumSecondSlash + 1, string::npos);
			//Second num stuff
			unsigned int secondNumSecondSlash = secondNum.find('/', secondNumFirstSlash + 1);
			string secondNumUV = secondNum.substr(secondNumFirstSlash + 1, secondNumSecondSlash - secondNumFirstSlash - 1);
			string secondNumNormal = secondNum.substr(secondNumSecondSlash + 1, string::npos);
			//Third num stuff
			unsigned int thirdNumSecondSlash = thirdNum.find('/', thirdNumFirstSlash + 1);
			string thirdNumUV = thirdNum.substr(thirdNumFirstSlash + 1, thirdNumSecondSlash - thirdNumFirstSlash - 1);
			string thirdNumNormal = thirdNum.substr(thirdNumSecondSlash + 1, string::npos);

			//...and into vertices.
			GenerateVertFromIndices(firstNumIndex,  firstNumNormal,  firstNumUV,  a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(secondNumIndex, secondNumNormal, secondNumUV, a_position, a_normal, a_uv, a_vertices, a_indices);
			GenerateVertFromIndices(thirdNumIndex,  thirdNumNormal,  thirdNumUV,  a_position, a_normal, a_uv, a_vertices, a_indices);
		}
	}
}

unsigned int Renderer::PositivifyIndex(const int index, const std::vector<vec3>* const a_position)
{
	if (index >= 0)
		return index;
	else
		return a_position->size() + index + 1;
}

unsigned int Renderer::PositivifyIndex(const int index, const std::vector<glm::vec2>* const a_uv)
{
	if (index >= 0)
		return index;
	else
		return a_uv->size() + index + 1;
}

void Renderer::GenerateVertFromIndices(const std::string& a_index, const std::string& a_normal, const std::string& a_uv,
									   const std::vector<vec3>* const a_positionVec, const std::vector<vec3>* const a_normalVec, const std::vector<glm::vec2>* const a_uvVec,
									   std::vector<Vertex>* const a_vertices,  std::vector<unsigned int>* const a_indices)
{
	Vertex vertex;
	if (a_index != "")
		vertex.position = vec4((*a_positionVec)[PositivifyIndex(std::stoi(a_index) - 1, a_positionVec)], 1);
	else
		vertex.position = vec4(0, 0, 0, 1);
	if (a_normal != "")
		vertex.normal = vec4((*a_normalVec)[PositivifyIndex(std::stoi(a_normal) - 1, a_normalVec)], 1);
	else
		vertex.normal = vec4(0, 1, 0, 1);
	if (a_uv != "")
		vertex.uv = glm::vec2((*a_uvVec)[PositivifyIndex(std::stoi(a_uv) - 1, a_uvVec)]);
	else
		vertex.uv = glm::vec2(0, 0);
	vertex.colour	= vec4(1, 1, 1, 1);
	vertex.tangent	= vec4(1, 0, 0, 1);

	std::vector<Vertex>::const_iterator index = std::find(a_vertices->begin(), a_vertices->end(), vertex);
	if (index == a_vertices->end())
	{
		a_vertices->push_back(vertex);
		a_indices->push_back(a_vertices->size() - 1);
	}
	else
	{
		a_indices->push_back(index - a_vertices->begin());
	}
}

void Renderer::LoadIntoOpenGL(const Vertex* const a_verticesArray, const unsigned int a_numOfVertices, const unsigned int* const a_indicesArray, const unsigned int a_numOfIndices, const bool a_animated)
{
	//Add the newest number of indices to the vector.
	m_numOfIndices.push_back(a_numOfIndices);

	//Add whether the object being loaded is animated.
	m_animated.push_back(a_animated);

	//Add a new empty texture and normal map if they haven't already been created.
	if (m_textures.size() < m_numOfIndices.size())
		m_textures.push_back(-1);
	if (m_normals.size() < m_numOfIndices.size())
		m_normals.push_back(-1);
	if (m_speculars.size() < m_numOfIndices.size())
		m_speculars.push_back(-1);

	//Add new buffer variables to the vectors.
	m_VAO.push_back(-1);
	m_VBO.push_back(-1);
	m_IBO.push_back(-1);

	//Generating buffers
	glGenVertexArrays(1, &m_VAO[m_VAO.size() - 1]);
	glGenBuffers(1, &m_VBO[m_VBO.size() - 1]);
	glGenBuffers(1, &m_IBO[m_IBO.size() - 1]);

	//Bind stuff
	glBindVertexArray(m_VAO[m_VAO.size() - 1]);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[m_VBO.size() - 1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[m_IBO.size() - 1]);

	//Enable positions, colours, normals, tangents, UVs, weights and indices
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);


	//Tell opengl how the memory is formatted
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colour));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, indices));

	//Tell openGL what the vertices and indices are.
	glBufferData(GL_ARRAY_BUFFER, a_numOfVertices * sizeof(Vertex), a_verticesArray, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, a_numOfIndices * sizeof(unsigned int), a_indicesArray, GL_STATIC_DRAW);

	//Unloading stuff.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::CleanupBuffers()
{
	for (unsigned int i = 0; i < m_VAO.size(); ++i)
	{
		glDeleteVertexArrays(1, &m_VAO[i]);
	}
	for (unsigned int i = 0; i < m_VBO.size(); ++i)
	{
		glDeleteBuffers(1, &m_VAO[i]);
	}
	for (unsigned int i = 0; i < m_IBO.size(); ++i)
	{
		glDeleteBuffers(1, &m_IBO[i]);
	}

	if (m_noTexturesProgram != -1)
	{
		glDeleteProgram(m_noTexturesProgram);
		if (m_noNormalsProgram != -1)
		{
			glDeleteProgram(m_noNormalsProgram);
			if (m_noSpecularsProgram != -1)
			{
				glDeleteProgram(m_noSpecularsProgram);
				if (m_standardProgram != -1)
				{
					glDeleteProgram(m_standardProgram);
					if (m_animatedProgram != -1)
					{
						glDeleteProgram(m_animatedProgram);
					}
				}
			}
		}
	}

	if (m_particleProgram != -1)
		glDeleteProgram(m_particleProgram);
}