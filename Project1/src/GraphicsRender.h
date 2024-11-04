#pragma once

#include "Renderer.h"
#include"model.h"
#include "Shader.h"
#include "Camera.h"
#include "SkinnedMeshRenderer.h"
#include "Mesh/MeshInstance.h"
struct ModelAndShader
{
	ModelAndShader(Model* _model, Shader* _shader)
	{
		this->model = _model;
		this->shader = _shader;
		this->model->modelShader = _shader;
	}
	Model* model;
	Shader* shader;
};

struct CompareDistances
{
	const glm::vec3& cameraPos;

	explicit CompareDistances(const glm::vec3& cameraPosition) : cameraPos(cameraPosition) {}

	bool operator()(const ModelAndShader* lhs, const ModelAndShader* rhs) const
	{
		glm::vec3 diff1 = cameraPos - lhs->model->transform.position;
		glm::vec3 diff2 = cameraPos - rhs->  model->transform.position;

		return glm::dot(diff2, diff2) < glm::dot(diff1, diff1);
	}
};




class GraphicsRender
{
public:
	GraphicsRender();
	~GraphicsRender();

	static GraphicsRender& GetInstance();


	void AddModelAndShader(Model* model, Shader* shader);
	void RemoveModel(Model* model);

	void SetCamera(Camera* camera);

	void SortObject();
	
	std::vector<Model*> GetModelList() const;

	void InitializeGraphics();
	void Draw();
	void Clear();
	void DrawSphere(glm::vec3 center, float radius, glm::vec4 color, bool isWireFrame = false);
	void DrawBox(glm::vec3 center, glm::vec3 dimensions, glm::vec4 color ,bool isWireFrame = false);
	void DrawBox(glm::vec3 center, glm::vec3 dimensions, glm::vec3 rotation, glm::vec4 color ,bool isWireFrame = false);
	void DrawLine(glm::vec3 start, glm::vec3 endpoint, glm::vec4 color);
	void ClearModelList();
	void CleanUpData();

	Model* selectedModel;
	Shader* selectedShader;
	Model* SkyBoxModel;
	Shader* defaultShader;
	Shader* solidColorShader;
	Shader* stencilShader;
	Shader* boneAnimationShader;
	Shader* alphaCutoutShader;
	Shader* alphaBlendShader;
	Shader* defaultInstanceShader;
	Shader* grassInstanceShader;
	Shader* particleShader;
	Shader* skyboxShader;
	void SetSelectedModel(Model* model);
	Model* GetSelectedModel();
	Camera* camera;
	bool isDebug = false;

private:
	




	std::vector<ModelAndShader*> modelAndShaderList;
	std::vector<ModelAndShader*> transparentmodelAndShaderList;


};

