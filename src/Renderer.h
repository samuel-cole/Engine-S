#ifndef RENDERER_H
#define RENDERER_H

#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include <vector>
#include <map>
#include <list>
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
	AMBIENT,
	NORMAL,
	SPECULAR,
	LIGHT_MATRIX,
	SHADOW_MAP,
	SHADOW_BIAS,
	MIRROR_MATRIX,
	VIEW
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
	//Stores the names of each loaded model and the index of that model's VAO within the m_VAO vector.
	//TODO: Support model reuse for FBX models with multiple meshes per model.
	std::map<std::string, unsigned int>  m_modelNames;

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

	//Stores the names of each texture and the OpenGL handle to that texture.
	std::map<std::string, unsigned int> m_textureNames;

	//Vector containing all of the textures (diffuse maps) associated with this renderer.
	std::vector<unsigned int> m_textures;
	//Vector containing all of the ambient maps associated with this renderer. NOTE: Ambient maps are only used while in deferred rendering mode.
	std::vector<unsigned int> m_ambients;
	//Vector containing all of the normal maps associated with this renderer.
	std::vector<unsigned int> m_normals;
	//Vector containing all of the specular maps associated with this renderer.
	std::vector<unsigned int> m_speculars;
	//Vector containing the camera that each mirror uses for rendering.
	std::vector<unsigned int> m_mirrors;

	//Default black ambient used for objects without an ambient map.
	unsigned int m_defaultAmbient;
	//Default grey diffuse used for objects without a diffuse map.
	unsigned int m_defaultDiffuse;
	//Default normal map used for objects without a normal map.
	unsigned int m_defaultNormal;
	//Default specular map used for objects without a specular map.
	unsigned int m_defaultSpec;
	//Default shadow map used for situations in which a shadow map hasn't been generated.
	unsigned int m_defaultShadow;

	//Vector containing all of the framebuffers associated with this renderer.
	std::vector<unsigned int> m_frameBuffers;
	//Vector containing all of the render buffors used for storing depth- this vector is only used for cleaning up purposes at the moment.
	std::vector<unsigned int> m_renderBuffers;
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

	//Whether or not the directional light should be on.
	float m_dirLightStrength;
	//The direction that light is coming from.
	vec3 m_lightDir;
	//The colour of the light.
	vec3 m_lightColour;
	//How powerful the specular component of the light should be.
	float m_specPow;

	//Vector that stores the vertex buffer object used for the point lights.
	unsigned int m_pointVBO;
	//Vector that stores the index buffer object for the point lights.
	unsigned int m_pointIBO;
	//Vector that stores the vertex array object for the point lights.
	unsigned int m_pointVAO;
	//Vector that stores all of the colours for the point lights.
	std::list<vec3> m_pointColours;
	//Vector that stores the positions of all of the point lights.
	std::list<vec3> m_pointPositions;
	//Vector that stores the radii for each of the point lights.
	std::list<float> m_pointRadii;

	//These variables are used to set up the G-Pass.
	unsigned int m_gpassFBO, m_diffuseTexture, m_ambientTexture, m_positionTexture, m_normalTexture, m_specularTexture, m_gpassDepth;
	//These variables are used to set up the light pass.
	unsigned int m_lightFBO, m_lightDiffuseTexture, m_lightSpecularTexture;
	//These are the programs used for deferred rendering.
	unsigned int m_gpassProgram, m_gpassAnimProgram, m_dirLightProgram, m_pointLightProgram, m_compositeProgram;
	//Bool to store whether everything is currently being rendered with forwards rendering or deferred rendering. True for deferred.
	bool m_deferredRenderMode;

	//Sets up a GPass framebuffer.
	void SetupGpass();
	//Sets up everything required for the light pass of deferred rendering.
	void SetupLightBuffer();
	//Sets up point lights for use.
	void SetupPointLights();

	//Used in drawing directional lights for deferred rendering.
	void DrawDirectionalLight();
	//Used in drawing point lights for deferred rendering.
	void DrawPointLights();

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
	void DrawModels(const unsigned int a_cameraIndex);

	//Generates the normals for the model at the index passed in.
	void GenerateNormals(const unsigned int a_index);

public:
	//Constructor for creating a new renderer.
	Renderer(Camera* const a_camera, TwBar* const a_bar);

	//Creates a new frame buffer. Returns the texture that is generated. Dimensions sets the size of the texture that is ouputted.
	unsigned int LoadFrameBuffer(Camera* const a_camera, const vec4& a_dimensions, const vec3& a_backgroundColour);

	//Creates a shadow map. Setting light width to a high number gives a large area that shadows can be created within, while setting it to a low number generates higher quality shadow maps.
	void GenerateShadowMap(const float a_lightWidth);
	//Generates a perlin noise map. Pass the index of the model to have a perlin-based transform added to it into a_index. Note that perlin maps are not supported for animated models. a_octaves determines how bumpy the map will be.
	//a_heights is used for passing out the heights of each vertex on the object after being modified by the Perlin Noise map.
	void GeneratePerlinNoiseMap(const unsigned int a_rows, const unsigned int a_columns, const unsigned int a_octaves, const float a_amplitude, const float a_persistence, const unsigned int a_index, const unsigned int a_seed, const bool a_tileable, std::vector<float>& a_heights = std::vector<float>());

	//Method for loading in a texture. Pass the index of the model to be textured into a_index.
	void LoadTexture(const std::string& a_filePath, const unsigned int a_index);
	//Method for loading in a texture that has already been created (used in conjunction with LoadFrameBuffer to create render targets).
	void LoadTexture(const unsigned int a_textureIndex, const unsigned int a_index);
	//Method for lading in an ambient map. Pass the index of the model to have the ambient map applied to it into a_index. NOTE: AMBIENT MAPS ARE ONLY USED IN 'DEFERRED' RENDER MODE.
	void LoadAmbient(const std::string& a_filePath, const unsigned int a_index);
	//Method for loading in a normal map. Pass the index of the model to have the normal map applied to it into a_index.
	void LoadNormalMap(const std::string& a_filePath, const unsigned int a_index);
	//Method for loading in a specular map. Pass the index of the model to have the specular map applied to it into a_index.
	void LoadSpecularMap(const std::string& a_filePath, const unsigned int a_index);
	
	//Mutator method for the global transform matrices of each model. a_index refers to the index of the model to be transformed.
	void SetTransform(const glm::mat4& a_transform, const unsigned int a_index);
	//Accessor method for the global transform matrices of each model. a_index refers to the index of the model to get the transform of.
	const glm::mat4& GetTransform(const unsigned int a_index);

	//Generates a grid of vertices on the x-z plane with the specified number of rows and columns. Returns the index of the grid, for use in texturing/transform altering.
	unsigned int GenerateGrid(const unsigned int a_rows, const unsigned int a_columns);

	//Generates a point light of the specified colour and radius. Set a_debug to true to add the light to antweakbar. Returns an index to the point light.
	unsigned int CreatePointLight(const vec3& a_colour, const float a_radius, const bool a_debug, const vec3& a_position = vec3(0, 0, 0));

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
	void SetLightPosition(const unsigned int a_index, const vec3& a_position);
	//Sets the 'returnValue' argument to the position of the emitter at the specified index. Returns true if the index is valid, false if it isn't.
	bool GetLightPosition(const unsigned int a_index, vec3& a_returnValue);
	//Sets the colour of the emitter at the specified index to the colour indicated.
	void SetLightColour(const unsigned int a_index, const vec3& a_colour);

	//Sets the position of the emitter at the specified index to the position indicated.
	void SetEmitterPosition(const unsigned int a_index, const bool a_gpuBased, const vec3& a_position);
	//Sets the position of the emitter at the specified index to the position indicated. Overload for setting the area in which particles spawn in- particles will spawn between the two points indicated. Only works for gpu particles.
	void SetEmitterPosition(const unsigned int a_index, const bool a_gpuBased, const vec3& a_position, const vec3& a_position2);
	//Returns the position of the emitter at the specified index.
	const vec3& GetEmitterPosition(const unsigned int a_index, const bool a_gpuBased);

	//Method for destroying an emitter- takes in the index of the emitter, and whether it was GPU based or not.
	void DestroyEmitter(const unsigned int a_emitterIndex, const bool a_gpuBased);

	//Function for loading an FBX model. Returns the index of the model.
	//TODO: Include a variable for ambients.
	//TODO: Make this work better with multiple meshes. Particularly, make sure that files with multiple meshes get correctly saved out for re-loading.
	//TODO: Rework this function. Why do paths for textures need to be specified here, when for objs they are loaded seperately? Potentially return a vector of unsigned ints for each object loaded in.
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

	//Destroys the object at the specified index. TODO: Make new objects check for -1 values within the buffer vectors and place themselves there, instead of pushing back new values to the vector.
	void DestroyObject(const unsigned int a_index);

	//Cleans up by deleting all OpenGL buffers and programs currently in use.
	void CleanupBuffers();

	//Switches between deferred and forward rendering.
	void SwitchDeferred();
};

#endif