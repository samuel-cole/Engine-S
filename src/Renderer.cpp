#include "Renderer.h"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "glm\ext.hpp"
#include "Camera.h"
#include "FBXFile.h"
#include "Particle.h"

#include <stb_image.h>


#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

using glm::vec3;
using std::string;

Renderer::Renderer(Camera* a_camera, TwBar* a_bar) : m_camera(a_camera), m_bar(a_bar), m_file(nullptr),
													 m_standardProgram(-1), m_particleProgram(-1), m_noNormalsProgram(-1), m_animatedProgram(-1), m_noTexturesProgram(-1)
{
	//Fill the uniform locations vector with empty vcetors. 300 should be more than enough programs.
	m_uniformLocations.assign(300, std::vector<unsigned int>());

	m_lightColour = vec3(1, 1, 1);
	m_lightDir = vec3(1, 1, 1);
	m_specPow = 128.0f;

	TwAddVarRW(m_bar, "Light Colour", TW_TYPE_COLOR3F, &m_lightColour[0], "");
	TwAddVarRW(m_bar, "Light Direction", TW_TYPE_DIR3F, &m_lightDir[0], "");
	TwAddVarRW(m_bar, "Specular Power", TW_TYPE_FLOAT, &m_specPow, "");
}

unsigned int Renderer::CreateProgram(string a_vertPath, string a_fragPath)
{
	//Vertex Shader
	char* vsSource;
	std::ifstream vertShader(a_vertPath);
	string shaderLine;
	string buffer;

	while (std::getline(vertShader, shaderLine))
	{
		buffer.append(shaderLine);
		buffer.append("\n");
	}

	vsSource = new char[buffer.length() + 1];
	for (unsigned int i = 0; i < buffer.length(); ++i)
	{
		vsSource[i] = buffer[i];
	}
	vertShader.close();
	vsSource[buffer.length()] = '\0';

	//Fragment Shader
	char* fsSource;
	std::ifstream fragShader(a_fragPath);
	shaderLine = "";
	buffer = "";

	while (std::getline(fragShader, shaderLine))
	{
		buffer.append(shaderLine);
		buffer.append("\n ");
	}

	fsSource = new char[buffer.length() + 1];
	for (unsigned int i = 0; i < buffer.length(); ++i)
	{
		fsSource[i] = buffer[i];
	}
	fragShader.close();
	fsSource[buffer.length()] = '\0';

	int success = GL_FALSE;

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);

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

	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "ProjectionView"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Bones"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Global"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightDir"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "LightColour"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "CameraPos"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "SpecPow"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Diffuse"));
	m_uniformLocations[programID].push_back(glGetUniformLocation(programID, "Normal"));

	return programID;
}

//Stuff for loading in a texture. Pass false into a_channels for RGB, or true for RGBA.
void Renderer::LoadTexture(string a_filePath, bool a_channels)
{
	if (m_noNormalsProgram == -1)
	{
		m_noNormalsProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNorm.txt");
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_textures[m_textures.size() - 1]);
	glBindTexture(GL_TEXTURE_2D, m_textures[m_textures.size() - 1]);
	glTexImage2D(GL_TEXTURE_2D, 0, (a_channels) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (a_channels) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

//Stuff for loading in a normal map. Pass false into a_channels for RGB, or true for RGBA.
void Renderer::LoadNormalMap(string a_filePath, bool a_channels)
{
	//Check to see if a program that can handle normal maps has been created, and make one if it hasn't.
	if (m_standardProgram == -1)
	{
		m_standardProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/frag.txt");
	}

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(a_filePath.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_normals[m_normals.size() - 1]);
	glBindTexture(GL_TEXTURE_2D, m_normals[m_normals.size() - 1]);
	glTexImage2D(GL_TEXTURE_2D, 0, (a_channels) ? GL_RGBA : GL_RGB, imageWidth, imageHeight, 0, (a_channels) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Renderer::GenerateGrid(unsigned int a_rows, unsigned int a_columns)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNormNoTex.txt");
	}

	Vertex* aoVertices = new Vertex[ a_rows * a_columns];
	for (unsigned int r = 0; r < a_rows; ++r)
	{
		for (unsigned int c = 0; c < a_columns; ++c)
		{
			aoVertices[ r * a_columns + c].position = vec4((float)c, 0, (float)r, 1);
	
			//Creating an arbitrary colour.
			//vec3 colour(sinf( (c / (float)(a_columns - 1)) * (r / (float)(a_rows - 1))));
			vec3 colour(1,1,1);
	
			aoVertices[r * a_columns + c].colour = vec4(colour, 1);
			aoVertices[r * a_columns + c].normal = glm::vec4(0, 1, 0, 1);
			aoVertices[r * a_columns + c].tangent = glm::vec4(1, 0, 0, 1);
			aoVertices[r * a_columns + c].uv = glm::vec2((float)r / (a_rows / 10), (float)c / (a_columns / 10));
		}
	}
	
	unsigned int* auiIndices = new unsigned int[(a_rows - 1) * (a_columns - 1) * 6];

	unsigned int index = 0;
	for (unsigned int r = 0; r < (a_rows - 1); ++r)
	{
		for (unsigned int c = 0; c < (a_columns - 1); ++c)
		{
			//Triangle 1
			auiIndices[index++] = r * a_columns + c;
			auiIndices[index++] = (r + 1) * a_columns + c;
			auiIndices[index++] = (r + 1) * a_columns + (c + 1);

			//Triangle 2
			auiIndices[index++] = r * a_columns + c;
			auiIndices[index++] = (r + 1) * a_columns + (c + 1);
			auiIndices[index++] = r * a_columns + (c + 1); 
		}
	}

	LoadIntoOpenGL(aoVertices, a_rows * a_columns, auiIndices, (a_rows - 1) * (a_columns - 1) * 6, false);

	delete[] aoVertices;
	delete[] auiIndices;
}

unsigned int Renderer::CreateEmitter(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifespanMin, float a_lifespanMax,
									 float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const vec4& a_startColour, const vec4& a_endColour)
{
	if (m_particleProgram == -1)
	{
		m_particleProgram = CreateProgram("../data/shaders/vertParticles.txt", "../data/shaders/fragParticles.txt");
	}

	ParticleEmitter *emitter = new ParticleEmitter(a_maxParticles, a_emitRate, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startColour, a_endColour);
	
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

void Renderer::DestroyEmitter(unsigned int a_emitterIndex)
{
	if (m_emitters[a_emitterIndex] != nullptr)
	{
		delete m_emitters[a_emitterIndex];
		m_emitters[a_emitterIndex] = nullptr;
	}
}

void Renderer::LoadFBX(string a_filePath)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNormNoTex.txt");
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

		skeleton->evaluate(animation, 0);

		for (unsigned int i = 0; i < skeleton->m_boneCount; ++i)
		{
			skeleton->m_nodes[i]->updateGlobalTransform();
		}
	}
}

void Renderer::LoadFBX(string a_filePath, std::vector<string>* a_texturePaths, std::vector<string>* a_normalMapPaths, 
					   std::vector<bool>* a_texChannels, std::vector<bool>* a_normChannels)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNormNoTex.txt");
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
			LoadTexture((*a_texturePaths)[j], (*a_texChannels)[j]);
		}
		if (j < a_normalMapPaths->size())
		{
			LoadNormalMap((*a_normalMapPaths)[j], (*a_normChannels)[j]);
		}
	}


	if (m_file->getSkeletonCount() > 0)
	{
		if (m_animatedProgram == -1)
			m_animatedProgram = CreateProgram("../data/shaders/vertAnim.txt", "../data/shaders/frag.txt");

		FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);
		FBXAnimation* animation = m_file->getAnimationByIndex(0);

		skeleton->evaluate(animation, 0);

		for (unsigned int i = 0; i < skeleton->m_boneCount; ++i)
		{
			skeleton->m_nodes[i]->updateGlobalTransform();
		}
	}
}

void Renderer::LoadOBJ(string a_filePath)
{
	if (m_noTexturesProgram == -1)
	{
		m_noTexturesProgram = CreateProgram("../data/shaders/vert.txt", "../data/shaders/fragNoNormNoTex.txt");
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
}

void Renderer::Draw()
{
	if (m_noTexturesProgram != -1)
	{
		//I use noTexturesCheck to see if there are any models with no texture. 
		//I use an int instead of a bool so that I can start at the index of the first found object without either when I start drawing.
		unsigned int noTexturesCheck = -1;
		for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
		{
			if (m_textures[i] == -1 && m_normals[i] == -1)
			{
				noTexturesCheck = i;
				break;
			}
		}

		if (noTexturesCheck != -1)
		{
			glUseProgram(m_noTexturesProgram);

			glUniformMatrix4fv((m_uniformLocations[m_noTexturesProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

			glUniform3f((m_uniformLocations[m_noTexturesProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
			glUniform3f((m_uniformLocations[m_noTexturesProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
			glUniform3f((m_uniformLocations[m_noTexturesProgram])[CAMERA_POS], m_camera->GetWorldTransform()[3].x, m_camera->GetWorldTransform()[3].y, m_camera->GetWorldTransform()[3].z);
			glUniform1f((m_uniformLocations[m_noTexturesProgram])[SPEC_POW], m_specPow);

			for (unsigned int i = noTexturesCheck; i < m_numOfIndices.size(); ++i)
			{
				if (m_textures[i] != -1 || m_normals[i] != -1)
					continue;

				//Draw stuff
				glBindVertexArray(m_VAO[i]);
				glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
			}
		}


		//No normals program can only have a value if a no texture program has already been created, hence this is nested.
		if (m_noNormalsProgram != -1)
		{
			//I use noTexturesCheck to see if there are any models with no texture. 
			//I use an int instead of a bool so that I can start at the index of the first found object without either when I start drawing.
			unsigned int noNormalsCheck = -1;
			for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
			{
				if (m_textures[i] != -1 && m_normals[i] == -1)
				{
					noNormalsCheck = i;
					break;
				}
			}

			if (noNormalsCheck != -1)
			{
				glUseProgram(m_noNormalsProgram);

				glUniformMatrix4fv((m_uniformLocations[m_noNormalsProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

				glUniform3f((m_uniformLocations[m_noNormalsProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
				glUniform3f((m_uniformLocations[m_noNormalsProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
				glUniform3f((m_uniformLocations[m_noNormalsProgram])[CAMERA_POS], m_camera->GetWorldTransform()[3].x, m_camera->GetWorldTransform()[3].y, m_camera->GetWorldTransform()[3].z);
				glUniform1f((m_uniformLocations[m_noNormalsProgram])[SPEC_POW], m_specPow);

				for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
				{
					if (m_textures[i] == -1 || m_normals[i] != -1)
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
			if (m_standardProgram != -1)
			{
				//I use notAnimatedCheck to see if there are any models that aren't animated. 
				//I use an int instead of a bool so that I can start at the index of the first found object without either when I start drawing.
				unsigned int notAnimatedCheck = -1;
				for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
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

					glUniformMatrix4fv((m_uniformLocations[m_standardProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

					glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
					glUniform3f((m_uniformLocations[m_standardProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
					glUniform3f((m_uniformLocations[m_standardProgram])[CAMERA_POS], m_camera->GetWorldTransform()[3].x, m_camera->GetWorldTransform()[3].y, m_camera->GetWorldTransform()[3].z);
					glUniform1f((m_uniformLocations[m_standardProgram])[SPEC_POW], m_specPow);

					for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
					{
						if (m_textures[i] == -1 || m_normals[i] == -1 || m_animated[i])
							continue;

						// Set Texture/Diffuse Slot
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, m_textures[i]);
						glUniform1i((m_uniformLocations[m_standardProgram])[DIFFUSE], 0);

						// Set Normal Slot
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, m_normals[i]);
						glUniform1i((m_uniformLocations[m_standardProgram])[NORMAL], 1);

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

					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));
					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[BONES], skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);
					glUniformMatrix4fv((m_uniformLocations[m_animatedProgram])[GLOBAL], 1, GL_FALSE, &(m_file->getMeshByIndex(0)->m_globalTransform[0][0]));

					glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
					glUniform3f((m_uniformLocations[m_animatedProgram])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
					glUniform3f((m_uniformLocations[m_animatedProgram])[CAMERA_POS], m_camera->GetWorldTransform()[3].x, m_camera->GetWorldTransform()[3].y, m_camera->GetWorldTransform()[3].z);
					glUniform1f((m_uniformLocations[m_animatedProgram])[SPEC_POW], m_specPow);

					for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
					{
						if (m_animated[i] == false)
							continue;

						//At the moment, only textured models with normal maps are supported for animation, so continue if this isn't the case.
						if (m_textures[i] == -1 || m_normals[i] == -1)
							continue;

						// Set Texture/Diffuse Slot
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, m_textures[i]);
						glUniform1i((m_uniformLocations[m_animatedProgram])[DIFFUSE], 0);

						// Set Normal Slot
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, m_normals[i]);
						glUniform1i((m_uniformLocations[m_animatedProgram])[NORMAL], 1);

						//Draw stuff
						glBindVertexArray(m_VAO[i]);
						glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
					}
				}
			}
		}
	}

	//Particles are seperate from the huge mess of nested 'if' statements above because they are not dependant on any types of models being created.
	if (m_particleProgram != -1)
	{
		glUseProgram(m_particleProgram);

		glUniformMatrix4fv((m_uniformLocations[m_particleProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

		for (unsigned int i = 0; i < m_emitters.size(); ++i)
		{
			m_emitters[i]->Draw();
		}
	}


	/*//Old code below here, delete later.
	glUseProgram(m_programID);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Bind the Camera
	glUniformMatrix4fv((m_uniformLocations[m_programID])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));
	//Skeleton
	if (m_file != nullptr && m_file->getSkeletonCount() > 0)
	{
		FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);
		glUniformMatrix4fv((m_uniformLocations[m_programID])[BONES], skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);

		glUniformMatrix4fv((m_uniformLocations[m_programID])[GLOBAL], 1, GL_FALSE, &(m_file->getMeshByIndex(0)->m_globalTransform[0][0]));
	}
	// Light Direction
	glUniform3f((m_uniformLocations[m_programID])[LIGHT_DIR], m_lightDir.x, m_lightDir.y, m_lightDir.z);
	// Light Colour
	glUniform3f((m_uniformLocations[m_programID])[LIGHT_COLOUR], m_lightColour.x, m_lightColour.y, m_lightColour.z);
	// Camera Position
	glUniform3f((m_uniformLocations[m_programID])[CAMERA_POS], m_camera->GetWorldTransform()[3].x, m_camera->GetWorldTransform()[3].y, m_camera->GetWorldTransform()[3].z);
	// Specular Power
	glUniform1f((m_uniformLocations[m_programID])[SPEC_POW], m_specPow);

	for (unsigned int i = 0; i < m_numOfIndices.size(); ++i)
	{
		// Texture/Diffuse Stuff
		if (i < m_textures.size())
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textures[i]);

			glUniform1i((m_uniformLocations[m_programID])[DIFFUSE], 0);
		}
		// Set Normal Slot
		if (i < m_normals.size())
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_normals[i]);

			glUniform1i((m_uniformLocations[m_programID])[NORMAL], 1);
		}

		//Draw stuff
		glBindVertexArray(m_VAO[i]);
		glDrawElements(GL_TRIANGLES, m_numOfIndices[i], GL_UNSIGNED_INT, nullptr);
	}

	if (m_particleProgram != -1)
	{
		glUseProgram(m_particleProgram);

		glUniformMatrix4fv((m_uniformLocations[m_particleProgram])[PROJECTION_VIEW], 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

		for (unsigned int i = 0; i < m_emitters.size(); ++i)
		{
			m_emitters[i]->Draw();
		}
	}*/
}

void Renderer::UpdateAnimation(float a_time)
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

void Renderer::UpdateEmitters(float a_deltaTime)
{
	for (unsigned int i = 0; i < m_emitters.size(); ++i)
	{
		m_emitters[i]->Update(a_deltaTime, m_camera->GetWorldTransform());
	}
}

void Renderer::SplitIndex(string a_string, std::vector<Vertex>* a_vertices, std::vector<unsigned int>* a_indices, std::vector<vec3>* a_position, std::vector<vec3>* a_normal, std::vector<glm::vec2>* a_uv)
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

unsigned int Renderer::PositivifyIndex(int index, std::vector<vec3>* a_position)
{
	if (index >= 0)
		return index;
	else
		return a_position->size() + index + 1;
}

unsigned int Renderer::PositivifyIndex(int index, std::vector<glm::vec2>* a_uv)
{
	if (index >= 0)
		return index;
	else
		return a_uv->size() + index + 1;
}

void Renderer::GenerateVertFromIndices(std::string a_index, std::string a_normal, std::string a_uv,
										 std::vector<vec3>* a_positionVec, std::vector<vec3>* a_normalVec, std::vector<glm::vec2>* a_uvVec,
										 std::vector<Vertex>* a_vertices, std::vector<unsigned int>* a_indices)
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

	std::vector<Vertex>::iterator index = std::find(a_vertices->begin(), a_vertices->end(), vertex);
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

void Renderer::LoadIntoOpenGL(Vertex *a_verticesArray, unsigned int a_numOfVertices, unsigned int *a_indicesArray, unsigned int a_numOfIndices, bool a_animated)
{
	//Add the newest number of indices to the vector.
	m_numOfIndices.push_back(a_numOfIndices);
	
	//Add whether the object being loaded is animated.
	m_animated.push_back(a_animated);

	//Add a new texture and normal map.
	m_textures.push_back(-1);
	m_normals.push_back(-1);

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
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)* 2));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)* 3));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)* 4));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)* 4 + sizeof(glm::vec2)));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)* 5 + sizeof(glm::vec2)));

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

	if (m_particleProgram != -1)
		glDeleteProgram(m_particleProgram);
}