#include "Renderer.h"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "glm\ext.hpp"
#include "Camera.h"
#include "StaticCamera.h"
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

void TW_CALL ChangeDeferred(void* a_clientData)
{
	Renderer* renderer = (Renderer*)a_clientData;
	renderer->SwitchDeferred();
}

Renderer::Renderer(Camera* const a_camera, TwBar* const a_bar) : m_bar(a_bar), m_shadowDepth(-1),
m_standardProgram(-1), m_particleProgram(-1), m_animatedProgram(-1), m_postProcessingProgram(-1), m_shadowGenProgram(-1), m_animShadowGenProgram(-1)
{
	//Fill the uniform locations vector with empty vcetors. 300 should be more than enough programs.
	m_uniformLocations.assign(300, std::vector<unsigned int>());

	m_lightColour = vec3(1, 1, 1);
	m_lightDir = vec3(1, -1, 1);
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

	LoadIntoOpenGL(vertexArray, 4, indexArray, 6);
	LoadTexture(texture, 0);

	m_postProcessingProgram = CreateProgram("../data/shaders/vertPostProcessing.txt", "../data/shaders/fragPostProcessing.txt");

	m_defaultDiffuse = LoadTexture("../data/default/specular.png");
	m_defaultNormal = LoadTexture("../data/default/normal.png");
	m_defaultShadow = LoadTexture("../data/default/shadow.png");
	m_defaultSpec = m_defaultShadow;
	
	m_deferredRenderMode = true;

	/////////Deferred Rendering Stuff//////////
	SetupGpass();
	SetupLightBuffer();
	SetupPointLights();
	m_dirLightProgram = CreateProgram("../data/shaders/vertPostProcessing.txt", "../data/shaders/fragLightDir.txt");
	m_pointLightProgram = CreateProgram("../data/shaders/vertLightPoint.txt", "../data/shaders/fragLightPoint.txt");
	m_compositeProgram = CreateProgram("../data/shaders/vertPostProcessing.txt", "../data/shaders/fragComposite.txt");
	TwAddButton(a_bar, "Switch Deferred/Forward Rendering", ChangeDeferred, (void*)this, "");
	/////////End of Deferred Rendering Stuff///

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::SetupGpass()
{
	//Frame buffer stuff.
	glGenFramebuffers(1, &m_gpassFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gpassFBO);

	//Set up albedo.
	glGenTextures(1, &m_albedoTexture);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 1280, 720);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Set up position texture.
	glGenTextures(1, &m_positionTexture);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1280, 720);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Set up normal texture.
	glGenTextures(1, &m_normalTexture);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1280, 720);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Render buffer stuff.
	glGenRenderbuffers(1, &m_gpassDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_gpassDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);

	
	//In case I forget : a frame buffer is essentially just a collection of pointers to render buffers and textures.Render buffers are actual collections of pixels.
	//Set textures for the frame buffer.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_albedoTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_positionTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_normalTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_gpassDepth);

	GLenum gpassTargets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(3, gpassTargets);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_gpassProgram = CreateProgram("../data/shaders/vertGbuffer.txt", "../data/shaders/fragGbuffer.txt");
}

void Renderer::SetupLightBuffer()
{
	//Light framebuffer
	glGenFramebuffers(1, &m_lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);

	//Light texture
	glGenTextures(1, &m_lightTexture);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 1280, 720);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightTexture, 0);

	GLenum lightTargets[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, lightTargets);

	unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Error: Light Framebuffer generation failed!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupPointLights()
{
	float cubeVertexData[] =
	{
		-1, -1, 1, 1,
		1, -1, 1, 1,
		1, -1, -1, 1,
		-1, -1, -1, 1,
		-1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, -1, 1,
		-1, 1, -1, 1,
	};

	unsigned int cubeIndexData[] =
	{
		0, 5, 4,
		0, 1, 5,
		1, 6, 5,
		1, 2, 6,
		2, 7, 6,
		2, 3, 7,
		3, 4, 7,
		3, 0, 4,
		4, 6, 7,
		4, 5, 6,
		3, 1, 0,
		3, 2, 1
	};

	//Generating buffers
	glGenVertexArrays(1, &m_pointVAO);
	glGenBuffers(1, &m_pointVBO);
	glGenBuffers(1, &m_pointIBO);

	//Bind stuff
	glBindVertexArray(m_pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pointIBO);

	//Enable positions only
	glEnableVertexAttribArray(0);

	//Tell opengl how the memory is formatted
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);

	//Tell openGL what the vertices and indices are.
	glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(vec4), cubeVertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(vec4), cubeIndexData, GL_STATIC_DRAW);

	//Unloading stuff.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Renderer::SwitchDeferred()
{
	m_deferredRenderMode = !m_deferredRenderMode;
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
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];

		glGetProgramInfoLog(programID, infoLogLength, 0, infoLog);
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
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "TransposeInverseGlobal"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightDir"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightColour"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "CameraPos"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "SpecPow"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Diffuse"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Normal"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Specular"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightMatrix"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "ShadowMap"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "ShadowBias"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "MirrorMatrix"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "View"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "AlbedoTexture"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightTexture"));

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
	m_renderBuffers.push_back(fboDepth);
	m_frameBufferDimensions.push_back(a_dimensions);
	m_frameBufferColours.push_back(a_backgroundColour);
	
	m_cameras.push_back(a_camera);

	return FBOTexture;
}

void Renderer::GenerateShadowMap(const float a_lightWidth)
{
	if (m_shadowGenProgram == -1)
		m_shadowGenProgram = CreateProgram("../data/shaders/vertShadowMap.txt", "../data/shaders/fragShadowMap.txt");

	if (m_standardProgram == -1)
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");

	if (m_animShadowGenProgram == -1)
		m_animShadowGenProgram = CreateProgram("../data/shaders/vertShadowMapAnimated.txt", "..data/shaders/fragShadowMap.txt");

	//Generate framebuffer to store shadow map in.
	glGenFramebuffers(1, &m_shadowMap);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMap);

	glGenTextures(1, &m_shadowDepth);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepth);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Capture depth- not colour.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadowDepth, 0);

	glDrawBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Error: Shadow map framebuffer was not created correctly." << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_lightProjection = glm::ortho<float>(-a_lightWidth, a_lightWidth, -a_lightWidth, a_lightWidth, -a_lightWidth, a_lightWidth);
}

void Renderer::GeneratePerlinNoiseMap(const unsigned int a_rows, const unsigned int a_columns, const unsigned int a_octaves, const float a_amplitude, const float a_persistence, const unsigned int a_index, const unsigned int a_seed, const bool a_tileable)
{
	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Generating Perlin Map for invalid object!" << std::endl;
		return;
	}


	float *perlinData = new float[a_rows * a_columns];
	
	for (unsigned int i = 0; i < a_rows; ++i)
	{
		for (unsigned int j = 0; j < a_columns; ++j)
		{
			perlinData[i * a_columns + j] = 0;

			float amplitude = a_amplitude;


			for (unsigned int o = 0; o < a_octaves; ++o)
			{
				float frequency = powf(2, (float)o);

				float perlinSample;
				if (!a_tileable)
				{
					glm::vec2 perlinInput = glm::vec2((float)i, (float)j) * (1.0f / glm::max(a_rows, a_columns)) * 3 * frequency;
					perlinSample = glm::perlin(perlinInput + glm::vec2((float)a_seed));
				}
				else
				{
					float xPerlin;
					if (i > ((float)a_rows - 1.0f) / 2.0f)
					{
						xPerlin = (float)((a_rows - 1 - i) % 2);
					}
					else
					{
						xPerlin = (float)(i % 2);
					}

					float yPerlin;
					if (j > ((float)a_rows - 1.0f) / 2.0f)
					{
						yPerlin = (float)((a_columns - 1 - j) % 2);
					}
					else
					{
						yPerlin = (float)(j % 2);
					}
					 

					glm::vec2 perlinInput = glm::vec2(xPerlin, yPerlin)  * (1.0f / glm::max(a_rows, a_columns)) * 3 * frequency;

					//At the moment, repeating textures do not use a seed- fix this later.
					perlinSample = glm::perlin(perlinInput);
				}
				perlinSample *= 0.5f + 0.5f;
				perlinData[i * a_columns + j] += perlinSample * amplitude;
				amplitude *= a_persistence;
			}
		}
	}

	//Here I am going to change the positions of the vertices within the object by the amounts specified by the Perlin Noise results.
	//Get the vertex data out of the vbo.
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[a_index]);
	Vertex* const vertices = (Vertex* const)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	//Get the number of vertices from the vbo.
	int verticesSize;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &verticesSize);
	verticesSize /= sizeof(Vertex);
	
	for (int i = 0; i < verticesSize; ++i)
	{
		vertices[i].position += vertices[i].normal * perlinData[(unsigned int)(vertices[i].uv.x * (a_rows - 1)) * a_columns + (unsigned int)(vertices[i].uv.y * (a_columns - 1))];
	}
	
	//Cleanup the vertex buffer.
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GenerateNormals(a_index);
}

void Renderer::GenerateNormals(const unsigned int a_index)
{
	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Generating normals for invalid object!" << std::endl;
		return;
	}

	//Need to use glMapBuffer to get data from the vbo and ibo, then modify it.

	//First, bind the buffers necessary.

	glBindVertexArray(m_VAO[a_index]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[a_index]);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[a_index]);
	//Next, map the buffers.
	const unsigned int* const INDICES = (const unsigned int* const)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
	Vertex* const vertices = (Vertex* const)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	
	/////////////Here is where I calculate the new normals.
	
	//First up, get the size of the vertex buffer for use in for loops.
	int verticesSize;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &verticesSize);
	verticesSize /= sizeof(Vertex);
	
	//Now set all of the normals to the zero vector.
	for (int i = 0; i < verticesSize; ++i)
	{
		vertices[i].normal = vec4(0, 0, 0, 0);
	}
	
	//Here I create a new normal for each face and add its value to each vertex in it.
	for (unsigned int i = 0; i < m_numOfIndices[a_index]; i += 3)
	{
		vec4 faceNormal = vec4(glm::cross(vec3(vertices[INDICES[i + 1]].position) - vec3(vertices[INDICES[i]].position), vec3(vertices[INDICES[i + 2]].position) - vec3(vertices[INDICES[i]].position)), 0);
		vertices[INDICES[i]].normal += faceNormal;
		vertices[INDICES[i + 1]].normal += faceNormal;
		vertices[INDICES[i + 2]].normal += faceNormal;
	}
	
	//Finally, I normalize all of the normals.
	for (int i = 0; i < verticesSize; ++i)
	{
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
	
	/////////////End of new normals calculation.
	
	//Cleanup time now- unmap the buffers...
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	//... and then unbind them.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned int Renderer::LoadTexture(const std::string& a_path)
{
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_path.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	unsigned int textureIndex;

	glGenTextures(1, &textureIndex);
	glBindTexture(GL_TEXTURE_2D, textureIndex);
	glTexImage2D(GL_TEXTURE_2D, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	return textureIndex;
}

void Renderer::LoadTexture(const string& a_filePath, const unsigned int a_index)
{
	//while (a_index >= m_textures.size())
	//{
	//	m_textures.push_back(-1);
	//}

	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Loading texture for invalid object!" << std::endl;
		return;
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_textures[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_textures[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::LoadTexture(const unsigned int a_textureIndex, const unsigned int a_index)
{
	//while (a_index >= m_textures.size())
	//{
	//	m_textures.push_back(-1);
	//}

	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Loading texture for invalid object!" << std::endl;
		return;
	}

	m_textures[a_index] = a_textureIndex;
}

void Renderer::LoadNormalMap(const string& a_filePath, const unsigned int a_index)
{
	//while (a_index >= m_normals.size())
	//{
	//	m_normals.push_back(-1);
	//}

	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Loading normal map for invalid object!" << std::endl;
		return;
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_normals[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_normals[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::LoadSpecularMap(const string& a_filePath, const unsigned int a_index)
{
	//while (a_index >= m_speculars.size())
	//{
	//	m_speculars.push_back(-1);
	//}

	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Loading specular map for invalid object!" << std::endl;
		return;
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_speculars[a_index]);
	glBindTexture(GL_TEXTURE_2D, m_speculars[a_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (imageFormat == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::SetTransform(const mat4& a_transform, const unsigned int a_index)
{
	//while (a_index >= m_globals.size())
	//{
	//	m_globals.push_back(mat4());
	//}

	if (a_index >= m_numOfIndices.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: setting transform for invalid object!" << std::endl;
		return;
	}

	m_globals[a_index] = a_transform;
}

const mat4& Renderer::GetTransform(const unsigned int a_index)
{
	if (a_index >= m_globals.size() || m_numOfIndices[a_index] == -1)
	{
		std::cout << "Error: Getting transform for invalid object.";
		return m_globals[a_index];
	}
	else
		return m_globals[a_index];
}

unsigned int Renderer::GenerateGrid(const unsigned int a_rows, const unsigned int a_columns)
{
	if (m_standardProgram == -1)
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");
	

	unsigned int rows = a_rows + 1;
	unsigned int columns = a_rows + 1;

	Vertex* aoVertices = new Vertex[ rows * columns];
	for (unsigned int r = 0; r < rows; ++r)
	{
		for (unsigned int c = 0; c < columns; ++c)
		{
			aoVertices[ r * columns + c].position = vec4((float)c - a_columns/2, 0, (float)r - a_rows/2, 1);
			aoVertices[r * columns + c].colour = vec4(1, 1, 1, 1);
			aoVertices[r * columns + c].normal = glm::vec4(0, 1, 0, 0);
			aoVertices[r * columns + c].tangent = glm::vec4(1, 0, 1, 1);
			aoVertices[r * columns + c].uv = glm::vec2((float)(r + 1)/rows, (float)(c + 1)/columns);
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

	LoadIntoOpenGL(aoVertices, rows * columns, auiIndices, (rows - 1) * (columns - 1) * 6);

	delete[] aoVertices;
	delete[] auiIndices;

	return m_numOfIndices.size() - 1;
}

unsigned int Renderer::CreatePointLight(const vec3& a_colour, const float a_radius, const vec3& a_position)
{
	m_pointColours.push_back(a_colour);
	m_pointPositions.push_back(a_position);
	m_pointRadii.push_back(a_radius);

	TwAddSeparator(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size())).c_str(), "");
	TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" Colour")).c_str(), TW_TYPE_COLOR3F, &m_pointColours.back().x, "");
	TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" X-Pos")).c_str(), TW_TYPE_FLOAT, &m_pointPositions.back().x, "step=0.1");
	TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" Y-Pos")).c_str(), TW_TYPE_FLOAT, &m_pointPositions.back().y, "step=0.1");
	TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" Z-Pos")).c_str(), TW_TYPE_FLOAT, &m_pointPositions.back().z, "step=0.1");
	//TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" Position")).c_str(), TW_TYPE_DIR3F, &m_pointPositions[m_pointPositions.size() - 1].x, "");
	TwAddVarRW(m_bar, (std::string("Point Light ") + std::to_string(m_pointRadii.size()) + std::string(" Radius")).c_str(), TW_TYPE_FLOAT, &m_pointRadii.back(), "step=0.1");

	return m_pointPositions.size() - 1;
}

unsigned int Renderer::CreateEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax, const float a_velocityMin, const float a_velocityMax,
									 const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		GPUParticleEmitter* emitter = new GPUParticleEmitter(a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour, a_direction, a_directionVariance);

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

unsigned int Renderer::CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax, const float a_velocityMin, const float a_velocityMax,
									 const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased)
{
	if (a_gpuBased)
	{
		GPUParticleEmitter* emitter = new GPUParticleEmitter(a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour, a_direction, a_directionVariance);

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

unsigned int Renderer::CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax, const  float a_velocityMin, const float a_velocityMax,
	const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased, TwBar* a_bar)
{
	if (a_gpuBased)
	{
		GPUParticleEmitter* emitter = new GPUParticleEmitter(a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour, a_direction, a_directionVariance, a_bar, m_gpuEmitters.size());

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

unsigned int Renderer::LoadFBX(const string& a_filePath, const std::vector<string>* const a_texturePaths, const std::vector<string>* const a_normalMapPaths, const std::vector<string>* const a_specularMapPaths)
{
	if (m_standardProgram == -1)
	{
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");
	}

	FBXFile* file = new FBXFile();

	file->load(a_filePath.c_str());
	for (unsigned int j = 0; j < file->getMeshCount(); ++j)
	{
		FBXMeshNode* mesh = file->getMeshByIndex(j);
		file->initialiseOpenGLTextures();

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

		LoadIntoOpenGL(vertices, mesh->m_vertices.size(), indices, mesh->m_indices.size());

		SetTransform(mesh->m_globalTransform, m_numOfIndices.size() - 1);

		delete[] vertices;
		delete[] indices;

		if (j < a_texturePaths->size())
		{
			LoadTexture((*a_texturePaths)[j], m_numOfIndices.size() - 1);
		}
		if (j < a_normalMapPaths->size())
		{
			LoadNormalMap((*a_normalMapPaths)[j], m_numOfIndices.size() - 1);
		}
		if (j < a_specularMapPaths->size())
		{
			LoadSpecularMap((*a_specularMapPaths)[j], m_numOfIndices.size() - 1);
		}

		if (file->getSkeletonCount() > 0)
		{
			if (m_animatedProgram == -1)
				m_animatedProgram = CreateProgram("../data/shaders/vertAnim.txt", "../data/shaders/frag.txt");
			
			if (j < file->getSkeletonCount() && j < file->getAnimationCount())
			{
				m_skeletons[m_numOfIndices.size() - 1] = file->getSkeletonByIndex(j);
				m_animations[m_numOfIndices.size() - 1] = file->getAnimationByIndex(j);

				m_skeletons[m_numOfIndices.size() - 1]->updateBones();

				m_skeletons[m_numOfIndices.size() - 1]->evaluate(m_animations[m_numOfIndices.size() - 1], 0);

				for (unsigned int i = 0; i < m_skeletons[m_numOfIndices.size() - 1]->m_boneCount; ++i)
				{
					m_skeletons[m_numOfIndices.size() - 1]->m_nodes[i]->updateGlobalTransform();
				}
			}
		}
	}

	return m_numOfIndices.size() - file->getMeshCount();
}

unsigned int Renderer::LoadOBJ(const string& a_filePath)
{
	if (m_standardProgram == -1)
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");
	

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

	LoadIntoOpenGL(aoVertices, vertices.size(), auiIndices, indices.size());

	delete[] aoVertices;
	delete[] auiIndices;

	return m_numOfIndices.size() - 1;
}

unsigned int Renderer::MakeMirror(const unsigned int a_width, const unsigned int a_length, const vec4& a_dimensions, const vec3& a_backgroundColour)
{
	//Make reflection camera.
	StaticCamera* camera = new StaticCamera();
	camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	//camera->SetPerspectiveOrtho(-1024, 1024, -1024, 1024, 0.1f, 100000.0f);
	camera->SetLookAt(vec3(0, 0, 0), vec3(0, 0, 5), vec3(0, 1, 0));

	m_cameras.push_back(camera);

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
		std::cout << "Error: mirror creation failed" << std::endl;

	//Unbind framebuffer so that we can render back to the back buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_frameBuffers.push_back(FBO);
	m_frameBufferDimensions.push_back(a_dimensions);
	m_frameBufferColours.push_back(a_backgroundColour);
	
	//Make mirror
	unsigned int mirror = GenerateGrid(a_width, a_length);
	LoadTexture(FBOTexture, mirror);
	//Add mirror to ignore list for mirror's framebuffer.
	m_frameBufferIgnores.push_back(std::make_pair(m_frameBuffers.size() - 1, mirror));	

	while (mirror >= m_mirrors.size())
	{
		m_mirrors.push_back(-1);
	}
	m_mirrors[mirror] = m_cameras.size() - 1;

	return mirror;
}

void Renderer::Draw()
{

	if (!m_deferredRenderMode)
	{
#pragma region NOT_DEFERRED_CODE
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if (m_shadowGenProgram != -1)
		{
			//Render to the shadow map for non-animated objects.
			glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMap);
			glViewport(0, 0, 4096, 4096);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(m_shadowGenProgram);
			//TODO: change this so that the light is coming from the camera position.
			m_lightProjView = m_lightProjection * glm::lookAt(-m_lightDir, vec3(0, 0, 0), vec3(0, 1, 0));
			glUniformMatrix4fv((m_uniformLocations[m_shadowGenProgram])[LIGHT_MATRIX], 1, GL_FALSE, &(m_lightProjView[0][0]));

			for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
			{
				if (m_skeletons[i] == nullptr && m_numOfIndices[i] != -1)
				{
					glUniformMatrix4fv((m_uniformLocations[m_shadowGenProgram])[GLOBAL], 1, GL_FALSE, &((m_globals[i])[0][0]));

					glBindVertexArray(m_VAO[i]);
					glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
				}
			}

			//Render to the shadow map for animated objects.
			if (m_animatedProgram != -1)
			{
				glUseProgram(m_animShadowGenProgram);
				glUniformMatrix4fv((m_uniformLocations[m_animShadowGenProgram])[LIGHT_MATRIX], 1, GL_FALSE, &(m_lightProjView[0][0]));

				for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
				{
					//Don't need to check if this object is valid, as deleted objects have their skeleton set to nullptr, so this is already doing the check.
					if (m_skeletons[i] != nullptr)
					{
						glUniformMatrix4fv((m_uniformLocations[m_animShadowGenProgram])[BONES], m_skeletons[i]->m_boneCount, GL_FALSE, (float*)m_skeletons[i]->m_bones);
						glUniformMatrix4fv((m_uniformLocations[m_animShadowGenProgram])[GLOBAL], 1, GL_FALSE, &((m_globals[i])[0][0]));

						glBindVertexArray(m_VAO[i]);
						glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
					}
				}
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
#pragma endregion
	}
	else
	{
#pragma region DEFERRED_CODE
		///////////////////////////G-Pass\\\\\\\\\\\\\\\\\\\\\\\\
			//G-Pass: render out the albedo, position and normal.
		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, m_gpassFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_gpassProgram);

		glUniformMatrix4fv((m_uniformLocations[m_gpassProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[0]->GetProjectionView()[0][0]));
		glUniformMatrix4fv((m_uniformLocations[m_gpassProgram])[VIEW], 1, GL_FALSE, &(m_cameras[0]->GetView()[0][0]));

		for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
		{
			glUniformMatrix4fv((m_uniformLocations[m_gpassProgram])[GLOBAL], 1, GL_FALSE, &((m_globals[i])[0][0]));

			// Set Texture/Diffuse Slot
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ((m_textures[i] == -1) ? m_defaultDiffuse : m_textures[i]));
			glUniform1i((m_uniformLocations[m_gpassProgram])[DIFFUSE], 0);

			// Set Normal Map Slot
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, ((m_normals[i] == -1) ? m_defaultNormal : m_normals[i]));
			glUniform1i((m_uniformLocations[m_gpassProgram])[NORMAL], 1);

			glBindVertexArray(m_VAO[i]);
			glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
		}


		///////////////////////////LIGHT\\\\\\\\\\\\\\\\\\\\\\\\
		//Light Pass: render lights as geometry, sampling position and normals.
		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);
		glClear(GL_COLOR_BUFFER_BIT);

		//Disable depth testing and enable additive blending.
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		//Draw lights as fullscreen quads.
		DrawDirectionalLight();

		DrawPointLights();

		glDisable(GL_BLEND);

		///////////////////////////COMPOSITE\\\\\\\\\\\\\\\\\\\\\\\\
		//Composite Pass: render a quad and combine albedo and light.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_compositeProgram);

		//Albedo
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
		glUniform1i((m_uniformLocations[m_compositeProgram])[ALBEDO], 0);

		//Light
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_lightTexture);
		glUniform1i((m_uniformLocations[m_compositeProgram])[LIGHT], 1);

		//Draw
		glBindVertexArray(m_VAO[0]);
		glDrawElements(GL_TRIANGLES, m_numOfIndices[0], GL_UNSIGNED_INT, nullptr);



		//CPU Particles
		if (m_particleProgram != -1)
		{
			glUseProgram(m_particleProgram);

			glUniformMatrix4fv((m_uniformLocations[m_particleProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[0]->GetProjectionView()[0][0]));

			for (unsigned int i = 0; i < m_emitters.size(); ++i)
			{
				if (m_emitters[i] != nullptr)
					m_emitters[i]->Draw();
			}
		}

		//GPU Particles
		for (unsigned int i = 0; i < m_gpuEmitters.size(); ++i)
		{
			if (m_gpuEmitters[i] != nullptr)
			{
				m_gpuEmitters[i]->Draw((float)glfwGetTime(), m_cameras[0]->GetWorldTransform(), m_cameras[0]->GetProjectionView());
			}
		}
#pragma endregion
	}
}

void Renderer::DrawDirectionalLight()
{
	glUseProgram(m_dirLightProgram);

	//Pass in normals
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glUniform1i((m_uniformLocations[m_dirLightProgram])[NORMAL], 0);

	//Pass in positions
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glUniform1i((m_uniformLocations[m_dirLightProgram])[GLOBAL], 1);

	//vec4 viewSpaceLight = m_cameras[0]->GetView() * vec4(glm::normalize(m_lightDir), 0);
	
	//glUniform3f((m_uniformLocations[m_dirLightProgram])[LIGHT_DIR], viewSpaceLight.x, viewSpaceLight.y, viewSpaceLight.z);
	glUniform3f((m_uniformLocations[m_dirLightProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
	glUniform3f((m_uniformLocations[m_dirLightProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
	glUniform3fv((m_uniformLocations[m_dirLightProgram])[CAMERA_POS], 1, &(m_cameras[0]->GetWorldTransform()[3][0]));
	glUniform1f((m_uniformLocations[m_dirLightProgram])[SPEC_POW], m_specPow);

	glBindVertexArray(m_VAO[0]);
	glDrawElements(GL_TRIANGLES, m_numOfIndices[0], GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawPointLights()
{
	if (m_pointRadii.size() > 0)
	{
		glUseProgram(m_pointLightProgram);

		glCullFace(GL_FRONT);

		//Pass in normals
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_normalTexture);
		glUniform1i((m_uniformLocations[m_pointLightProgram])[NORMAL], 0);

		//Pass in positions
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_positionTexture);
		glUniform1i((m_uniformLocations[m_pointLightProgram])[GLOBAL], 1);


		//Uniforms that are uniform for all lights.
		glUniformMatrix4fv((m_uniformLocations[m_pointLightProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[0]->GetProjectionView()[0][0]));
		glUniform3fv((m_uniformLocations[m_pointLightProgram])[CAMERA_POS], 1, &(m_cameras[0]->GetWorldTransform()[3][0]));
		glUniform1f((m_uniformLocations[m_pointLightProgram])[SPEC_POW], m_specPow);


		std::list<vec3>::iterator pointColourIter = m_pointColours.begin();
		std::list<vec3>::iterator pointPositionsIter = m_pointPositions.begin();
		std::list<float>::iterator pointRadiiIter = m_pointRadii.begin();

		for (unsigned int i = 0; i < m_pointRadii.size(); ++i)
		{
			//std::cout << i << ": " << m_pointColours[m_pointColours.size() - 1].x << " " << m_pointColours[m_pointColours.size() - 1].y << " " << m_pointColours[m_pointColours.size() - 1].z << std::endl;

			//Uniforms that change per light.
			glUniform3fv((m_uniformLocations[m_pointLightProgram])[LIGHT_MATRIX], 1, &(pointPositionsIter->x));
			glUniform3fv((m_uniformLocations[m_pointLightProgram])[LIGHT_COLOUR], 1, &(pointColourIter->x));
			glUniform1f((m_uniformLocations[m_pointLightProgram])[LIGHT_DIR], (*pointRadiiIter));

			//Draw the light.
			glBindVertexArray(m_pointVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

			++pointColourIter;
			++pointPositionsIter;
			++pointRadiiIter;
		}

		glCullFace(GL_BACK);
	}
}

void Renderer::DrawModels(const unsigned int j)
{
	if (m_standardProgram != -1)
	{
		//I use notAnimatedCheck to see if there are any models that aren't animated. 
		//I use an int instead of a bool so that I can start at the index of the first found object that isn't animated when I start drawing.
		unsigned int notAnimatedCheck = -1;
		for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
		{
			if (m_skeletons[i] == nullptr && m_numOfIndices[i] != -1)
			{
				notAnimatedCheck = i;
				break;
			}
		}

		if (notAnimatedCheck != -1)
		{
			glUseProgram(m_standardProgram);

			mat4 lightMatrix = M_TEXTURE_SPACE_OFFSET * m_lightProjView;
			glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[LIGHT_MATRIX], 1, GL_FALSE, &(lightMatrix[0][0]));
			glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

			glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
			glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
			glUniform3f((m_uniformLocations[m_standardProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
			glUniform1f((m_uniformLocations[m_standardProgram])[SPEC_POW], m_specPow);
			if (m_shadowGenProgram != -1)
				glUniform1f((m_uniformLocations[m_standardProgram])[SHADOW_BIAS], 0.01f);
			else
				glUniform1f((m_uniformLocations[m_standardProgram])[SHADOW_BIAS], 1000.0f);

			for (unsigned int i = notAnimatedCheck; i < m_numOfIndices.size(); ++i)
			{
				if (m_skeletons[i] != nullptr || m_numOfIndices[i] == -1)
					continue;

				//Check to see if this model is on this frame buffer's ignore list.
				bool onIgnoreList = false;
				for (unsigned int k = 0; k < m_frameBufferIgnores.size(); ++k)
				{
					if (m_frameBufferIgnores[k].first == j && m_frameBufferIgnores[k].second == i)
					{
						onIgnoreList = true;
						break;
					}
				}
				if (onIgnoreList)
					continue;

				// Set Texture/Diffuse Slot
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ((m_textures[i] == -1) ? m_defaultDiffuse : m_textures[i]));
				glUniform1i((m_uniformLocations[m_standardProgram])[DIFFUSE], 0);

				// Set Normal Slot
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, ((m_normals[i] == -1) ? m_defaultNormal : m_normals[i]));
				glUniform1i((m_uniformLocations[m_standardProgram])[NORMAL], 1);

				// Set Specular Slot
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, ((m_speculars[i] == -1) ? m_defaultSpec : m_speculars[i]));
				glUniform1i((m_uniformLocations[m_standardProgram])[SPECULAR], 2);

				// Set Shadow Map Slot
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, ((m_shadowDepth == -1) ? m_defaultShadow : m_shadowDepth));
				glUniform1i((m_uniformLocations[m_standardProgram])[SHADOW_MAP], 3);

				// Set Transform
				glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[GLOBAL], 1, GL_FALSE, &((m_globals[i])[0][0]));
				mat4 transposeInverseGlobal = glm::transpose(glm::inverse(m_globals[i]));
				glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[TRANS_INV_GLOBAL], 1, GL_FALSE, &(transposeInverseGlobal[0][0]));

				//Draw stuff
				glBindVertexArray(m_VAO[i]);
				glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
			}
		}

		if (m_animatedProgram != -1)
		{
			//Here I check for animated models to draw.
			unsigned int animatedCheck = -1;
			for (unsigned int i = 1; i < m_numOfIndices.size(); ++i)
			{
				//Much like the Draw() function, I don't do checks to see if animated objects are invalid because I automatically set deleted objects m_skeleton entry to nullptr, hence this is already checked.
				if (m_skeletons[i] != nullptr)
				{
					animatedCheck = i;
					break;
				}
			}

			if (animatedCheck != -1)
			{
				glUseProgram(m_animatedProgram);

				mat4 lightMatrix = M_TEXTURE_SPACE_OFFSET * m_lightProjView;
				glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[LIGHT_MATRIX], 1, GL_FALSE, &(lightMatrix[0][0]));
				glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_cameras[j]->GetProjectionView()[0][0]));

				glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
				glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
				glUniform3f((m_uniformLocations[m_animatedProgram])[CAMERA_POS], m_cameras[j]->GetWorldTransform()[3].x, m_cameras[j]->GetWorldTransform()[3].y, m_cameras[j]->GetWorldTransform()[3].z);
				glUniform1f((m_uniformLocations[m_animatedProgram])[SPEC_POW], m_specPow);
				if (m_shadowGenProgram != -1)
					glUniform1f((m_uniformLocations[m_animatedProgram])[SHADOW_BIAS], 0.01f);
				else
					glUniform1f((m_uniformLocations[m_animatedProgram])[SHADOW_BIAS], 1000.0f);

				for (unsigned int i = animatedCheck; i < m_numOfIndices.size(); ++i)
				{
					if (m_skeletons[i] == nullptr)
						continue;

					//Check to see if this model is on this frame buffer's ignore list.
					bool onIgnoreList = false;
					for (unsigned int k = 0; k < m_frameBufferIgnores.size(); ++k)
					{
						if (m_frameBufferIgnores[k].first == j && m_frameBufferIgnores[k].second == i)
						{
							onIgnoreList = true;
							break;
						}
					}
					if (onIgnoreList)
						continue;

					// Set Texture/Diffuse Slot
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, ((m_textures[i] == -1) ? m_defaultDiffuse : m_textures[i]));
					glUniform1i((m_uniformLocations[m_standardProgram])[DIFFUSE], 0);

					// Set Normal Slot
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, ((m_normals[i] == -1) ? m_defaultNormal : m_normals[i]));
					glUniform1i((m_uniformLocations[m_standardProgram])[NORMAL], 1);

					// Set Specular Slot
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, ((m_speculars[i] == -1) ? m_defaultSpec : m_speculars[i]));
					glUniform1i((m_uniformLocations[m_standardProgram])[SPECULAR], 2);

					// Set Shadow Map Slot
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, ((m_shadowDepth == -1) ? m_defaultShadow : m_shadowDepth));
					glUniform1i((m_uniformLocations[m_standardProgram])[SHADOW_MAP], 3);

					// Set Transform
					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[GLOBAL], 1, GL_FALSE, &((m_globals[i])[0][0]));
					mat4 transposeInverseGlobal = glm::transpose(glm::inverse(m_globals[i]));
					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[TRANS_INV_GLOBAL], 1, GL_FALSE, &(transposeInverseGlobal[0][0]));
					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[BONES], m_skeletons[i]->m_boneCount, GL_FALSE, (float*)m_skeletons[i]->m_bones);


					//Draw stuff
					glBindVertexArray(m_VAO[i]);
					glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
				}
			}
		}
	}
		
	//CPU Particles
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

	//GPU Particles
	for (unsigned int i = 0; i < m_gpuEmitters.size(); ++i)
	{
		if (m_gpuEmitters[i] != nullptr)
		{
			m_gpuEmitters[i]->Draw((float)glfwGetTime(), m_cameras[j]->GetWorldTransform(), m_cameras[j]->GetProjectionView());
		}
	}
}

void Renderer::UpdateAnimation(const float a_time, const unsigned int a_index)
{
	if (m_skeletons[a_index] != nullptr)
	{
		m_skeletons[a_index]->updateBones();

		m_skeletons[a_index]->evaluate(m_animations[a_index], a_time);

		for (unsigned int i = 0; i < m_skeletons[a_index]->m_boneCount; ++i)
		{
			m_skeletons[a_index]->m_nodes[i]->updateGlobalTransform();
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

void Renderer::UpdateMirrors()
{
	for (unsigned int i = 0; i < m_mirrors.size(); ++i)
	{
		if (m_mirrors[i] != -1)
		{
			vec3 cameraPosition = vec3(m_cameras[0]->GetWorldTransform()[3]);
			vec3 mirrorPosition = vec3(m_globals[i][3]);
			vec3 incident = glm::normalize(mirrorPosition - cameraPosition);
			vec3 normal = glm::normalize(vec3(m_globals[i] * vec4(0, 1, 0, 0)));
			//The reflected vector is equal to Incident - 2 * (Incident.Normal) * Normal, http://www.cosinekitty.com/raytrace/chapter10_reflection.html has a good explanation of how this is derived.
			vec3 reflected = glm::normalize(incident - 2 * glm::dot(incident, normal) * normal);
			vec3 newCameraPos = reflected * -glm::length(mirrorPosition - cameraPosition);
			m_cameras[m_mirrors[i]]->SetLookAt(newCameraPos, mirrorPosition, vec3(0, 1, 0));
			//m_cameras[m_mirrors[i]]->SetLookAt(mirrorPosition, mirrorPosition + reflected, vec3(0, 1, 0));

		}
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
			//Double slash

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
		vertex.normal = vec4((*a_normalVec)[PositivifyIndex(std::stoi(a_normal) - 1, a_normalVec)], 0);
	else
		vertex.normal = vec4(0, 1, 0, 0);
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

void Renderer::LoadIntoOpenGL(const Vertex* const a_verticesArray, const unsigned int a_numOfVertices, const unsigned int* const a_indicesArray, const unsigned int a_numOfIndices)
{
	//Add the newest number of indices to the vector.
	m_numOfIndices.push_back(a_numOfIndices);

	//Add whether the object being loaded is animated.
	m_skeletons.push_back(nullptr);
	m_animations.push_back(nullptr);

	//Add a new empty entry into each of this objects appropriate vectors.
	if (m_textures.size() < m_numOfIndices.size())
		m_textures.push_back(-1);
	if (m_normals.size() < m_numOfIndices.size())
		m_normals.push_back(-1);
	if (m_speculars.size() < m_numOfIndices.size())
		m_speculars.push_back(-1);
	if (m_mirrors.size() < m_numOfIndices.size())
		m_mirrors.push_back(-1);
	if (m_globals.size() < m_numOfIndices.size())
		m_globals.push_back(glm::mat4());

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

void Renderer::DestroyObject(const unsigned int a_index)
{
	glDeleteVertexArrays(1, &m_VAO[a_index]);
	m_VAO[a_index] = -1;
	glDeleteBuffers(1, &m_VBO[a_index]);
	m_VBO[a_index] = -1;
	glDeleteBuffers(1, &m_IBO[a_index]);
	m_IBO[a_index] = -1;

	glDeleteTextures(1, &m_textures[a_index]);
	m_textures[a_index] = -1;
	glDeleteTextures(1, &m_normals[a_index]);
	m_normals[a_index] = -1;
	glDeleteTextures(1, &m_speculars[a_index]);
	m_speculars[a_index] = -1;

	m_skeletons[a_index] = nullptr;
	m_animations[a_index] = nullptr;
	m_globals[a_index] = mat4();

	m_numOfIndices[a_index] = -1;
}

void Renderer::CleanupBuffers()
{
	for (unsigned int i = 0; i < m_VAO.size(); ++i)
	{
		glDeleteVertexArrays(1, &m_VAO[i]);
	}
	for (unsigned int i = 0; i < m_VBO.size(); ++i)
	{
		glDeleteBuffers(1, &m_VBO[i]);
	}
	for (unsigned int i = 0; i < m_IBO.size(); ++i)
	{
		glDeleteBuffers(1, &m_IBO[i]);
	}


	for (unsigned int i = 0; i < m_renderBuffers.size(); ++i)
	{
		glDeleteRenderbuffers(1, &m_renderBuffers[i]);
	}
	for (unsigned int i = 0; i < m_frameBuffers.size(); ++i)
	{
		glDeleteFramebuffers(1, &m_frameBuffers[i]);
	}


	if (m_standardProgram != -1)
		glDeleteProgram(m_standardProgram);
	if (m_animatedProgram != -1)
		glDeleteProgram(m_animatedProgram);
	if (m_particleProgram != -1)
		glDeleteProgram(m_particleProgram);
	if (m_postProcessingProgram != -1)
		glDeleteProgram(m_postProcessingProgram);
	if (m_shadowGenProgram != -1)
		glDeleteProgram(m_shadowGenProgram);
	if (m_animShadowGenProgram != -1)
		glDeleteProgram(m_animShadowGenProgram);

	if (m_shadowDepth != -1)
		glDeleteTextures(1, &m_shadowDepth);

	for (unsigned int i = 0; i < m_textures.size(); ++i)
	{
		if (m_textures[i] != -1)
			glDeleteTextures(1, &m_textures[i]);
	}
	for (unsigned int i = 0; i < m_normals.size(); ++i)
	{
		if (m_normals[i] != -1)
			glDeleteTextures(1, &m_normals[i]);
	}
	for (unsigned int i = 0; i < m_speculars.size(); ++i)
	{
		if (m_speculars[i] != -1)
			glDeleteTextures(1, &m_speculars[i]);
	}

	glDeleteTextures(1, &m_defaultDiffuse);
	glDeleteTextures(1, &m_defaultNormal);
	glDeleteTextures(1, &m_defaultShadow);

	for (unsigned int i = 0; i < m_mirrors.size(); ++i)
	{
		if (m_mirrors[i] != -1)
			delete m_cameras[m_mirrors[i]];
	}
}