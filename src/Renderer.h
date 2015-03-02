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

	//Uniform Locations
	std::vector<std::vector<unsigned int>> m_uniformLocations;

	Camera* m_camera;

	TwBar* m_bar;

	FBXFile* m_file;
	
	std::vector<unsigned int> m_textures;
	std::vector<unsigned int> m_normals;

	std::vector<ParticleEmitter*> m_emitters;

	vec3 m_lightDir;
	vec3 m_lightColour;
	float m_specPow;

	void SplitIndex(std::string a_string,				  std::vector<Vertex>* a_vertices,
					std::vector<unsigned int>* a_indices, std::vector<vec3>* a_position,
					std::vector<vec3>* a_normal,		  std::vector<glm::vec2>* a_uv);

	//Generates a vertex from the indices passed in, and adds it to the vertex and index list past in if it doesn't already exist in it. If information isn't provided, pass in an empty string.
	void GenerateVertFromIndices(std::string a_index,				 std::string a_normal,			 std::string a_uv, 
								 std::vector<vec3>* a_positionVec,   std::vector<vec3>* a_normalVec, std::vector<glm::vec2>* a_uvVec,
								 std::vector<Vertex>* a_vertices,	 std::vector<unsigned int>* a_indices);

	unsigned int PositivifyIndex(int index, std::vector<vec3>* a_position);
	unsigned int PositivifyIndex(int index, std::vector<glm::vec2>* a_uv);

	void LoadIntoOpenGL(Vertex *a_verticesArray, unsigned int a_numOfVertices, unsigned int *a_indicesArray, unsigned int a_numOfIndices, bool a_animated);
public:

	Renderer(Camera* a_camera, TwBar* a_bar);
	unsigned int CreateProgram(std::string a_vertPath, std::string a_fragPath);

	//Stuff for loading in a texture. Pass false into a_channels for RGB, or true for RGBA. This has to be used AFTER loading the thing you want to texture.
	void LoadTexture(std::string a_filePath, bool a_channels);
	//Stuff for loading in a normal map. Pass false into a_channels for RGB, or true for RGBA. This has to be used AFTER loading the thing you want to add a normal map to.
	void LoadNormalMap(std::string a_filePath, bool a_channels);

	void GenerateGrid(unsigned int a_rows, unsigned int a_columns);

	unsigned int CreateEmitter(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifespanMin, float a_lifespanMax,
							   float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const vec4& a_startColour, const vec4& a_endColour);
	void DestroyEmitter(unsigned int a_emitterIndex);
	
	//Method for loading an FBX model without textures/normalmaps, or with only one mesh (if it has textures/normalmaps and only one mesh, LoadTexture/LoadNormalMap can be loaded seperately).
	void LoadFBX(std::string a_filePath);
	//Method for loading an FBX model with textures. Use this method instead of calling LoadTexture() seperately for FBX models with multiple meshes. Pass false into the channels for RGB, or true for RGBA.
	void LoadFBX(std::string a_filePath, std::vector<std::string>* a_texturePaths, std::vector<std::string>* a_normalMapPaths,
				 std::vector<bool>* a_texChannels, std::vector<bool>* a_normChannels);

	void LoadOBJ(std::string a_filePath);
	
	void Draw();

	void UpdateAnimation(float a_time);
	void UpdateEmitters(float a_deltaTime);

	void CleanupBuffers();

};

#endif