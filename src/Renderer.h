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
	NORMAL
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
	std::vector<unsigned int> m_VAO;
	std::vector<unsigned int> m_VBO;
	std::vector<unsigned int> m_IBO;

	//Vector containing the number of indices associated with each IBO.
	std::vector<unsigned int> m_numOfIndices;

	//Vector containing which buffers are animated.
	std::vector<bool> m_animated;

	//Program used for animated models.
	unsigned int m_animatedProgram;
	//Program used for standard models.
	unsigned int m_standardProgram;
	//Program used when a texture is included with the model, but no normal map.
	unsigned int m_noNormalsProgram;
	//Program used when no normal map or texture is included with the model.
	unsigned int m_noTexturesProgram;
	//Program used for particles.
	unsigned int m_particleProgram;
	/*Please note that programs for many situations are missing.
	Some missing ones include a program for animated models without textures and/or normal maps, and a program for models with a normal map but no texture*/

	//Uniform Locations. Indexed with program id, then uniform type.
	std::vector<std::vector<unsigned int>> m_uniformLocations;

	Camera* m_camera;

	TwBar* m_bar;

	FBXFile* m_file;
	
	std::vector<unsigned int> m_textures;
	std::vector<unsigned int> m_normals;

	std::vector<ParticleEmitter*> m_emitters;
	std::vector<GPUParticleEmitter*> m_gpuEmitters;

	vec3 m_lightDir;
	vec3 m_lightColour;
	float m_specPow;

	void SplitIndex(const std::string& a_string,		   std::vector<Vertex>* a_vertices,
					std::vector<unsigned int>* a_indices,  std::vector<vec3>* a_position,
					std::vector<vec3>* a_normal,		   std::vector<glm::vec2>* a_uv);

	//Generates a vertex from the indices passed in, and adds it to the vertex and index list past in if it doesn't already exist in it. If information isn't provided, pass in an empty string.
	void GenerateVertFromIndices(const std::string& a_index,				 const std::string& a_normal,			 const std::string& a_uv, 
								 const std::vector<vec3>* a_positionVec,    const std::vector<vec3>* a_normalVec,  const std::vector<glm::vec2>* a_uvVec,
								 std::vector<Vertex>* a_vertices,			 std::vector<unsigned int>* a_indices);

	unsigned int PositivifyIndex(const int index, const std::vector<vec3>* a_position);
	unsigned int PositivifyIndex(const int index, const std::vector<glm::vec2>* a_uv);

	unsigned int LoadShader(const std::string& a_path, unsigned int a_type);

	void LoadIntoOpenGL(const Vertex *a_verticesArray, const unsigned int a_numOfVertices, const unsigned int *a_indicesArray, const unsigned int a_numOfIndices, const bool a_animated);
public:

	Renderer(Camera* a_camera, TwBar* a_bar);

	unsigned int CreateProgram(const std::string& a_vertPath, const std::string& a_fragPath);

	//Stuff for loading in a texture. Pass false into a_channels for RGB, or true for RGBA. This has to be used AFTER loading the thing you want to texture.
	void LoadTexture(const std::string& a_filePath, const bool a_channels);
	//Stuff for loading in a normal map. Pass false into a_channels for RGB, or true for RGBA. This has to be used AFTER loading the thing you want to add a normal map to.
	void LoadNormalMap(const std::string& a_filePath, const bool a_channels);

	void GenerateGrid(const unsigned int a_rows, const unsigned int a_columns);

	unsigned int CreateEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax,
							   const  float a_velocityMin, const float a_velocityMax, const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, bool a_gpuBased);

	void DestroyEmitter(const unsigned int a_emitterIndex, const bool a_gpuBased);
	
	//Method for loading an FBX model without textures/normalmaps, or with only one mesh (if it has textures/normalmaps and only one mesh, LoadTexture/LoadNormalMap can be loaded seperately).
	void LoadFBX(const std::string& a_filePath);
	//Method for loading an FBX model with textures. Use this method instead of calling LoadTexture() seperately for FBX models with multiple meshes. Pass false into the channels for RGB, or true for RGBA.
	void LoadFBX(const std::string& a_filePath, const std::vector<std::string>* a_texturePaths, const std::vector<std::string>* a_normalMapPaths,
				 const std::vector<bool>* a_texChannels, const std::vector<bool>* a_normChannels);

	void LoadOBJ(const std::string& a_filePath);
	
	void Draw();

	void UpdateAnimation(const float a_time);
	void UpdateEmitters(const float a_deltaTime);

	void CleanupBuffers();

};

#endif