#ifndef RENDERER_H
#define RENDERER_H

#include "glm\glm.hpp"
#include <vector>
#include <map>
#include "AntTweakBar.h"

using glm::vec4;
using glm::vec3;

class Camera;
class FBXFile;
class ParticleEmitter;
class GPUParticleEmitter;

enum UniformTypes
{
	PROJECTION_VIEW,
	BONES,
	GLOBAL,
	LIGHT_DIR,
	LIGHT_COLOUR,
	CAMERA_POS,
	SPEC_POW,
	DIFFUSE,
	NORMAL,
	SPECULAR
};

struct Vertex
{
	//Used in all vertices
	vec4 position;
	vec4 colour;
	vec4 normal;
	vec4 tangent;
	glm::vec2 uv;

	//Just used in FBX models.
	vec4 weights;
	vec4 indices;

	bool operator == (const Vertex& a_other)
	{
		return (a_other.position == position && a_other.normal == normal && a_other.uv == uv && a_other.colour == colour && a_other.tangent == tangent);
	}
};

class Renderer
{
private:
	//Vector of vertex array objects.
	std::vector<unsigned int> m_VAO;
	//Vector of vertex buffer objects.
	std::vector<unsigned int> m_VBO;
	//Vector of index buffer objects.
	std::vector<unsigned int> m_IBO;

	//Vector containing the number of indices associated with each IBO.
	std::vector<unsigned int> m_numOfIndices;

	//Vector containing which buffers are animated.
	std::vector<bool> m_animated;

	//Program used for animated models.
	unsigned int m_animatedProgram;
	//Program used for standard models.
	unsigned int m_standardProgram;
	//Program used when a texture and normal map is included with the model, but no specular map.
	unsigned int m_noSpecularsProgram;
	//Program used when a texture is included with the model, but no normal map.
	unsigned int m_noNormalsProgram;
	//Program used when no normal map or texture is included with the model.
	unsigned int m_noTexturesProgram;
	//Program used for particles.
	unsigned int m_particleProgram;
	//Program used for post-processing.
	unsigned int m_postProcessingProgram;
	/*Please note that programs for many situations are missing.
	Some missing ones include a program for animated models without textures, specular and/or normal maps, and a program for models with a normal map but no texture*/

	//Uniform Locations. Indexed with program id, then uniform type.
	std::vector<std::vector<unsigned int>> m_uniformLocations;

	//Vector containing all of the textures associated with this renderer.
	std::vector<unsigned int> m_textures;
	//Vector containing all of the normal maps associated with this renderer.
	std::vector<unsigned int> m_normals;
	//Vector containing all of the specular maps associated with this renderer.
	std::vector<unsigned int> m_speculars;

	//Vector containing all of the framebuffers associated with this renderer.
	std::vector<unsigned int> m_frameBuffers;
	//Vector containing the positions and sizes of all of the frame buffers associated with this renderer.
	std::vector<vec4> m_frameBufferDimensions;
	//Vector containing the background colours of each frame buffer. 
	std::vector<vec3> m_frameBufferColours;

	//Vector containing all of the CPU based particle emitters associated with this renderer.
	std::vector<ParticleEmitter*> m_emitters;
	//Vector containing all of the GPU based particle emitters associated with this renderer.
	std::vector<GPUParticleEmitter*> m_gpuEmitters;

	//Vector containing all of the cameras in the scene. There should be one per framebuffer- each framebuffer will use the camera at the same index as it.
	std::vector<Camera*> m_cameras;

	//HUD bar used for debugging.
	TwBar* m_bar;

	//Pointer to the FBX file being used. TODO: Change this to work with multiple FBXFiles.
	FBXFile* m_file;

	//The direction that light is coming from.
	vec3 m_lightDir;
	//The colour of the light.
	vec3 m_lightColour;
	//How powerful the specular component of the light should be.
	float m_specPow;

	//Splits an OBJ face(index) line into its components, and uses those components to construct vertices which it pushes into the appropriate vectors.
	void SplitIndex(const std::string& a_string,				std::vector<Vertex>* const a_vertices,
					std::vector<unsigned int>* const a_indices, std::vector<vec3>* const a_position,
					std::vector<vec3>* const a_normal,		    std::vector<glm::vec2>* const a_uv);

	//Generates a vertex from the indices passed in, and adds it to the vertex and index list passed in if it doesn't already exist in it. If information isn't provided, pass in an empty string.
	void GenerateVertFromIndices(const std::string& a_index,					const std::string& a_normal,				const std::string& a_uv, 
								 const std::vector<vec3>* const a_positionVec,  const std::vector<vec3>* const a_normalVec, const std::vector<glm::vec2>* const a_uvVec,
								 std::vector<Vertex>* const a_vertices,			std::vector<unsigned int>* const a_indices);

	//Converts the current OBJ position/normal index to its positive equivalent.
	unsigned int PositivifyIndex(const int index, const std::vector<vec3>* const a_positionOrNormal);
	//Converts the current OBJ uv index to its positive equivalent.
	unsigned int PositivifyIndex(const int index, const std::vector<glm::vec2>* const a_uv);

	//Loads in the file passed in as path, then uses it to create a new OpenGL buffer.
	unsigned int LoadShader(const std::string& a_path, unsigned int a_type);

	//Makes all of the necessary OpenGL calls to load the arrays of vertices and indices passed in to OpenGL.
	void LoadIntoOpenGL(const Vertex* const a_verticesArray, const unsigned int a_numOfVertices, const unsigned int* const a_indicesArray, const unsigned int a_numOfIndices, const bool a_animated);

	//Creates a new OpenGL program from the shaders passed in. Returns the index of the program.
	unsigned int CreateProgram(const std::string& a_vertPath, const std::string& a_fragPath);

public:
	//Constructor for creating a new renderer.
	Renderer(Camera* const a_camera, TwBar* const a_bar);

	//Creates a new frame buffer. Returns the index of the frame buffer.
	unsigned int LoadFrameBuffer(Camera* const a_camera, const vec4& a_dimensions, const vec3& a_backgroundColour, unsigned int& a_texture);

	//Method for loading in a texture. Pass false into a_channels for RGB, or true for RGBA. Pass the index of the model to be textured into a_index.
	void LoadTexture(const std::string& a_filePath, const bool a_channels, unsigned int a_index);
	//Method for loading in a texture that has already been created (used in conjunction with LoadFrameBuffer to create render targets).
	void LoadTexture(const unsigned int a_textureIndex, const unsigned int a_index);
	//Method for loading in a normal map. Pass false into a_channels for RGB, or true for RGBA.  Pass the index of the model to have the normal map applied to it into a_index.
	void LoadNormalMap(const std::string& a_filePath, const bool a_channels, unsigned int a_index);
	//Method for loading in a specular map. Pass false into a_channels for RGB, or true for RGBA. Pass the index of the model to have the specular map applied to it into a_index.
	void LoadSpecularMap(const std::string& a_filePath, const bool a_channels, unsigned int a_index);
	
	//Generates a grid of vertices on the x-z plane with the specified number of rows and columns. Returns the index of the grid, for use in texturing.
	unsigned int GenerateGrid(const unsigned int a_rows, const unsigned int a_columns);

	//Method for creating a particle emitter. Note that the emit rate variable will not be used if gpu-based particles are created.
	unsigned int CreateEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax,
							   const  float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const bool a_gpuBased);
	//Method for creating GPU-based particles only. Still has a bool to check gpu-based just to make sure that the correct function is being called.
	unsigned int CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax,
							   const  float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const bool a_gpuBased);

	//Sets the position of the emitter at the specified index to the position indicated.
	void SetEmitterPosition(const unsigned int a_index, const bool a_gpuBased, const vec3& a_position);
	//Returns the position of the emitter at thye specified index.
	const vec3& GetEmitterPosition(const unsigned int a_index, const bool a_gpuBased);

	//Method for destroying an emitter- takes in the index of the emitter, and whether it was GPU based or not.
	void DestroyEmitter(const unsigned int a_emitterIndex, const bool a_gpuBased);
	
	//Method for loading an FBX model without textures/normalmaps, or with only one mesh (if it has textures/normalmaps and only one mesh, LoadTexture/LoadNormalMap can be loaded seperately). Returns the index of the model, for use in texturing.
	unsigned int LoadFBX(const std::string& a_filePath);
	//Method for loading an FBX model with textures. Use this method instead of calling LoadTexture() seperately for FBX models with multiple meshes. Pass false into the channels for RGB, or true for RGBA.
	//Note that this method may not work well if used in conjunction with with the LoadTexture and LoadNormalMap functions- this already loads textures and normal maps, so you don't need to call those functions as well.
	void LoadFBX(const std::string& a_filePath, const std::vector<std::string>* const a_texturePaths, const std::vector<std::string>* const a_normalMapPaths, const std::vector<std::string>* const a_specularMapPaths,
				 const std::vector<bool>* const a_texChannels, const std::vector<bool>* const a_normChannels, const std::vector<bool>* const a_specularChannels);
	
	//Method for loading an OBJ model. Returns the index of the model, for use in texturing.
	unsigned int LoadOBJ(const std::string& a_filePath);
	
	//Draw method- does all drawing for all models and particles.
	void Draw();

	//Updates all animated FBX files to the time specified with a_time. TODO: Allow the time for each animation to be set seperately. To do this currently, you must have multiple renderer instances.
	void UpdateAnimation(const float a_time);
	//Updates all CPU-based particle emitters (and, by extension, all CPU-based particles).
	void UpdateEmitters(const float a_deltaTime);

	//Cleans up by deleting all OpenGL buffers and programs currently in use.
	void CleanupBuffers();
};

#endif