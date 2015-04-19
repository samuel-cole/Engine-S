#ifndef RENDERER_H
#define RENDERER_H

#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include <vector>
#include <map>
#include "AntTweakBar.h"

using glm::vec4;
using glm::vec3;

class Camera;
class FBXSkeleton;
class FBXAnimation;
class ParticleEmitter;
class GPUParticleEmitter;

enum UniformTypes
{
	PROJECTION_VIEW,
	BONES,
	GLOBAL,
	TRANS_INV_GLOBAL,
	LIGHT_DIR,
	LIGHT_COLOUR,
	CAMERA_POS,
	SPEC_POW,
	DIFFUSE,
	NORMAL,
	SPECULAR,
	LIGHT_MATRIX,
	SHADOW_MAP,
	SHADOW_BIAS,
	PERLIN_MAP,
	MIRROR_MATRIX
};

struct Vertex
{
	//TODO: change these to vec3s- the fourth variable i just being dropped in the vert shader anyway.
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

	//Vector containing the skeletons of animated objects. Set to nullptr for non-animated models.
	std::vector<FBXSkeleton*> m_skeletons;
	//Vector containing the animations of animated objects. Set to nullptr for non-animated models.
	std::vector<FBXAnimation*> m_animations;

	//Vector containing the world transforms of every object.
	std::vector<glm::mat4> m_globals;

	//TODO: change these programs to use an enum. 
	//TODO: add additional programs for different combinations of shadows and maps.
	//Program used for animated models.
	unsigned int m_animatedProgram;
	//Program used for standard models.
	unsigned int m_standardProgram;
	//Program used for particles.
	unsigned int m_particleProgram;
	//Program used for post-processing.
	unsigned int m_postProcessingProgram;
	//Program used for generating shadow maps.
	unsigned int m_shadowGenProgram;
	//Program used for generating shadow maps for animated objects.
	unsigned int m_animShadowGenProgram;

	//Uniform Locations. Indexed with program id, then uniform type.
	std::vector<std::vector<unsigned int>> m_uniformLocations;

	//Vector containing all of the textures associated with this renderer.
	std::vector<unsigned int> m_textures;
	//Vector containing all of the normal maps associated with this renderer.
	std::vector<unsigned int> m_normals;
	//Vector containing all of the specular maps associated with this renderer.
	std::vector<unsigned int> m_speculars;
	//Vector containing all of the perlin maps associated with this renderer.
	std::vector<unsigned int> m_perlins;
	//Vector containing the camera that each mirror uses for rendering.
	std::vector<unsigned int> m_mirrors;

	//Default black diffuse used for objects without a diffuse map.
	unsigned int m_defaultDiffuse;
	//Default normal map used for objects without a normal map.
	unsigned int m_defaultNormal;
	//Default specular map used for objects without a specular map.
	unsigned int m_defaultSpec;
	//Default shadow map used for situations in which a shadow map hasn't been generated.
	unsigned int m_defaultShadow;
	//Default perlin map used for situations in which a perlin map hasn't been generated.
	unsigned int m_defaultPerlin;

	//Vector containing all of the framebuffers associated with this renderer.
	std::vector<unsigned int> m_frameBuffers;
	//Vector containing the positions and sizes of all of the frame buffers associated with this renderer.
	std::vector<vec4> m_frameBufferDimensions;
	//Vector containing the background colours of each frame buffer. 
	std::vector<vec3> m_frameBufferColours;

	//Stores which frame buffers should ignore which objects- first unsigned int is for the frame buffer that is going to ignore an object, second one is for the object to be ignored.
	std::vector<std::pair<unsigned int, unsigned int>> m_frameBufferIgnores;

	//Vector containing indices to the m_frameBuffers vector- shows which framebuffers are used for post processing.
	//std::vector<unsigned int> m_postProcessingBufferIndices;

	//Handle to the texture that stores the shadow map
	unsigned int m_shadowMap;
	//Handle to the depth of the shadow map.
	unsigned int m_shadowDepth;
	//The projection matrix for lights, used in creating a shadow map.
	//TODO: change this to update as the camera changes position- should be smaller as the camera gets closer to the plane being illuminated.
	glm::mat4 m_lightProjection;
	//Matrix used to offset the light matrix while using a shadow map.
	const glm::mat4 M_TEXTURE_SPACE_OFFSET = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
													   0.0f, 0.5f, 0.0f, 0.0f,
													   0.0f, 0.0f, 0.5f, 0.0f,
													   0.5f, 0.5f, 0.5f, 1.0f);
	//Matrix the projection view matrix of the light used in shadow mapping.
	glm::mat4 m_lightProjView;

	//Vector containing all of the CPU based particle emitters associated with this renderer.
	std::vector<ParticleEmitter*> m_emitters;
	//Vector containing all of the GPU based particle emitters associated with this renderer.
	std::vector<GPUParticleEmitter*> m_gpuEmitters;

	//Vector containing all of the cameras in the scene. There should be one per framebuffer- each framebuffer will use the camera at the same index as it.
	std::vector<Camera*> m_cameras;

	//HUD bar used for debugging.
	TwBar* m_bar;

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

	//Loads in the texture with the specified path, and returns a handle to that texture.
	unsigned int LoadTexture(const std::string& a_path);

	//Makes all of the necessary OpenGL calls to load the arrays of vertices and indices passed in to OpenGL.
	void LoadIntoOpenGL(const Vertex * const a_verticesArray, const unsigned int a_numOfVertices, const unsigned int* const a_indicesArray, const unsigned int a_numOfIndices);

	//Creates a new OpenGL program from the shaders passed in. Returns the index of the program.
	unsigned int CreateProgram(const std::string& a_vertPath, const std::string& a_fragPath);

	//Method called by draw for drawing models. Pass in the index of the camera from which the models should be viewed.
	//TODO: Refactor to use a series of enum types to determine which types (normals, textures, shadows, etc.) are turned on.
	void DrawModels(unsigned int a_cameraIndex);

public:
	//Constructor for creating a new renderer.
	Renderer(Camera* const a_camera, TwBar* const a_bar);

	//Creates a new frame buffer. Returns the texture that is generated. Dimensions sets the size of the texture that is ouputted.
	unsigned int LoadFrameBuffer(Camera* const a_camera, const vec4& a_dimensions, const vec3& a_backgroundColour);

	//Creates a shadow map. Setting light width to a high number gives a large area that shadows can be created within, while setting it to a low number generates higher quality shadow maps.
	void GenerateShadowMap(const float a_lightWidth);
	//Generates a perlin noise map. Pass the index of the model to be perlined into a_index. Note that perlin maps are currently not supported for animated models. a_octaves determines how bumpy the map will be.
	//TODO: Generate normals for the perlin map. Good info here http://stackoverflow.com/questions/10922752/calculate-normals-for-procedural-shape
	void GeneratePerlinNoiseMap(const unsigned int a_rows, const unsigned int a_columns, const unsigned int a_octaves, const float a_amplitude, const float a_persistence, const unsigned int a_index, const unsigned int a_seed);

	//Method for loading in a texture. Pass the index of the model to be textured into a_index.
	void LoadTexture(const std::string& a_filePath, const unsigned int a_index);
	//Method for loading in a texture that has already been created (used in conjunction with LoadFrameBuffer to create render targets).
	void LoadTexture(const unsigned int a_textureIndex, const unsigned int a_index);
	//Method for loading in a normal map. Pass the index of the model to have the normal map applied to it into a_index.
	void LoadNormalMap(const std::string& a_filePath, const unsigned int a_index);
	//Method for loading in a specular map. Pass the index of the model to have the specular map applied to it into a_index.
	void LoadSpecularMap(const std::string& a_filePath, const unsigned int a_index);
	
	//Mutator method for the global transform matrices of each model. a_index refers to the index of the model to be transformed.
	void SetTransform(const glm::mat4& a_transform, const unsigned int a_index);
	//Accessor method for the global transform matrices of each model. a_index refers to the index of the model to get the transform of.
	const glm::mat4& GetTransform(const unsigned int a_index);

	//Generates a grid of vertices on the x-z plane with the specified number of rows and columns. Returns the index of the grid, for use in texturing.
	unsigned int GenerateGrid(const unsigned int a_rows, const unsigned int a_columns);

	//Method for creating a particle emitter. Note that the emit rate variable will not be used if gpu-based particles are created, and that the direction and directionvariance variables will not be used for cpu particles.
	unsigned int CreateEmitter(const unsigned int a_maxParticles, const unsigned int a_emitRate, const float a_lifespanMin, const float a_lifespanMax, const  float a_velocityMin, const float a_velocityMax, 
							   const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased);
	//Method for creating GPU-based particles only. Still has a bool to check gpu-based just to make sure that the correct function is being called.
	unsigned int CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax, const  float a_velocityMin, const float a_velocityMax,
							   const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased);
	//Method for creating GPU-based particles only. Still has a bool to check gpu-based just to make sure that the correct function is being called. Debug mode, includes a debug bar to change variables.
	unsigned int CreateEmitter(const unsigned int a_maxParticles, const float a_lifespanMin, const float a_lifespanMax, const  float a_velocityMin, const float a_velocityMax,
							   const float a_startSize, const float a_endSize, const vec4& a_startColour, const vec4& a_endColour, const vec3& a_direction, const float a_directionVariance, const bool a_gpuBased, TwBar* a_bar);

	//Sets the position of the emitter at the specified index to the position indicated.
	void SetEmitterPosition(const unsigned int a_index, const bool a_gpuBased, const vec3& a_position);
	//Returns the position of the emitter at thye specified index.
	const vec3& GetEmitterPosition(const unsigned int a_index, const bool a_gpuBased);

	//Method for destroying an emitter- takes in the index of the emitter, and whether it was GPU based or not.
	void DestroyEmitter(const unsigned int a_emitterIndex, const bool a_gpuBased);
	
	//Function for loading an FBX model. Returns the index of the model.
	unsigned int LoadFBX(const std::string& a_filePath, const std::vector<std::string>* const a_texturePaths, const std::vector<std::string>* const a_normalMapPaths, const std::vector<std::string>* const a_specularMapPaths);
	
	//Method for loading an OBJ model. Returns the index of the model, for use in texturing.
	unsigned int LoadOBJ(const std::string& a_filePath);

	//Creates a new frame buffer. Returns the texture that is generated. Dimensions sets the size of the texture that is ouputted.
	unsigned int MakeMirror(const unsigned int a_width, const unsigned int a_length, const vec4& a_dimensions, const vec3& a_backgroundColour);
	
	//Draw method- does all drawing for all models and particles to all framebuffers.
	void Draw();

	//Updates the animated FBX file specified by a_index to the time specified with a_time.
	void UpdateAnimation(const float a_time, const unsigned int a_index);
	//Updates all CPU-based particle emitters (and, by extension, all CPU-based particles).
	void UpdateEmitters(const float a_deltaTime);
	//Updates all mirrors to properly reflect.
	void UpdateMirrors();

	//Cleans up by deleting all OpenGL buffers and programs currently in use.
	void CleanupBuffers();
};

#endif