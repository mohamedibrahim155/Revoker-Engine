#include "GraphicsRender.h"
#include "DebugModels.h"
#include "EntityManager/EntityManager.h"
GraphicsRender::GraphicsRender()
{
}

GraphicsRender::~GraphicsRender()
{
	CleanUpData();
}

GraphicsRender& GraphicsRender::GetInstance()
{
	static GraphicsRender instance;
	return instance;
}


void GraphicsRender::AddModelAndShader(Model* model, Shader* shader)
{
	if (shader->blendMode ==OPAQUE || shader->blendMode == ALPHA_CUTOUT || shader->blendMode==SOLID)
	{
		modelAndShaderList.push_back(new ModelAndShader(model, shader));
	}
	else if (shader->blendMode == ALPHA_BLEND)
	{
		transparentmodelAndShaderList.push_back(new ModelAndShader(model, shader));
	}
	
}

void GraphicsRender::RemoveModel(Model* _model)
{
	for (ModelAndShader* model :  modelAndShaderList)
	{
		if (model->model == _model)
		{
			//delete model->model;
			//delete model->shader;

			modelAndShaderList.erase(std::remove(modelAndShaderList.begin(), modelAndShaderList.end(), model), 
				modelAndShaderList.end());
			return;
		}
	}

	for (ModelAndShader* model : transparentmodelAndShaderList)
	{
		if (model->model == _model)
		{
			//delete model->model;
			//delete model->shader;

			transparentmodelAndShaderList.erase(std::remove(transparentmodelAndShaderList.begin(), transparentmodelAndShaderList.end(), model),
				transparentmodelAndShaderList.end());
			return;
		}
	}
}


void GraphicsRender::SetCamera(Camera* camera)
{

	this->camera = camera;
}

void GraphicsRender::SortObject()
{
	CompareDistances compareDistance(camera->transform.position);
	std::sort(transparentmodelAndShaderList.begin(), transparentmodelAndShaderList.end(), compareDistance);


}

std::vector<Model*> GraphicsRender::GetModelList() const
{
	std::vector<ModelAndShader*> list = modelAndShaderList;
	std::vector<Model*> modelList;

	modelList.reserve(modelAndShaderList.size() + transparentmodelAndShaderList.size());
	
	for (unsigned int i = 0; i < list.size(); i++)
	{
		modelList.push_back(list[i]->model);
	}

	list = transparentmodelAndShaderList;

	for (unsigned int i = 0; i < list.size(); i++)
	{
		modelList.push_back(list[i]->model);
	}


	return modelList;
}





void GraphicsRender::InitializeGraphics()
{
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glEnable(GL_STENCIL_TEST));
	GLCALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
	GLCALL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void GraphicsRender::Draw()
{
	glDepthFunc(GL_LEQUAL);
	SkyBoxModel->Draw(skyboxShader);
	glDepthFunc(GL_LESS);

	//glStencilMask(0x00);

	for (ModelAndShader* modelAndShader : modelAndShaderList)
	{
		if (modelAndShader->model == selectedModel)  continue;

		modelAndShader->model->Draw(modelAndShader->shader);
	}



	SortObject();

	for (ModelAndShader* modelAndShader :  transparentmodelAndShaderList)
	{
		if (modelAndShader->model == selectedModel)  continue;
		modelAndShader->model->Draw(modelAndShader->shader);
	}


	if (selectedModel != nullptr)
	{
	
		// First pass: Render the model normally and write to the stencil buffer
		GLCALL(glStencilFunc(GL_ALWAYS, 1, 0xFF));
		GLCALL(glStencilMask(0xFF));
		selectedModel->Draw(selectedModel->modelShader);

		// Second pass: Render the model with a slightly larger scale to create an outline
		GLCALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
		GLCALL(glStencilMask(0x00));
		GLCALL(glDisable(GL_DEPTH_TEST));

		glm::vec3 tempScale = selectedModel->transform.scale;

		selectedModel->transform.scale.x += selectedModel->transform.scale.x * 0.01f;
		selectedModel->transform.scale.y += selectedModel->transform.scale.y * 0.01f;
		selectedModel->transform.scale.z += selectedModel->transform.scale.z * 0.01f;

		//selectedModel->transform.SetScale(tempScale * 1.025f);

		// Draw the model with the stencil shader
		selectedModel->DrawSolidColor(glm::vec4(0.75f,0.25f,0,1));

		// Restore the original scale
		selectedModel->transform.SetScale(tempScale);

		// Enable depth test and clear stencil buffer
		GLCALL(glEnable(GL_DEPTH_TEST));
		GLCALL(glStencilMask(0xFF));
		GLCALL(glStencilFunc(GL_ALWAYS, 0, 0xFF));
	}
	

}

void GraphicsRender::Clear()
{
	GLCALL(glClearColor(0.1f, 0.1f, 0.1f, 1));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	GLCALL(glStencilMask(0x00));
}

void GraphicsRender::DrawSphere(glm::vec3 center, float radius, glm::vec4 color, bool isWireFrame)
{
	
		Model* model = new Model(*(DebugModels::GetInstance().defaultSphere), true);

		model->transform.SetPosition(center);
		model->transform.SetScale(glm::vec3(radius));
		model->DrawSolidColor(color, isWireFrame);
		delete model;
	

}

void GraphicsRender::DrawBox(glm::vec3 center, glm::vec3 dimensions, glm::vec4 color, bool isWireFrame)
{
	
		Model* model = new Model(*(DebugModels::GetInstance().defaultCube), true);
		model->transform.SetPosition(center);
		model->transform.SetScale(dimensions);
		model->DrawSolidColor(color, isWireFrame);
		delete model;
}

void GraphicsRender::DrawBox(glm::vec3 center, glm::vec3 dimensions, glm::vec3 rotation, glm::vec4 color, bool isWireFrame)
{
	
		Model* model = new Model(*(DebugModels::GetInstance().defaultCube), true);
		model->transform.SetPosition(center);
		model->transform.SetRotation(rotation);
		model->transform.SetScale(dimensions);
		model->DrawSolidColor(color, isWireFrame);
		delete model;
}

void GraphicsRender::DrawLine(glm::vec3 start, glm::vec3 endpoint, glm::vec4 color)
{
	
		std::vector<Vertex> vertices =
		{
			 {glm::vec3(-1.0f, -1.0f, -1.0f)},  // 0
			{glm::vec3(1.0f, -1.0f, -1.0f)},  // 1
			{glm::vec3(1.0f, -1.0f, -1.0f)},  // 2
		};

		std::vector<unsigned int>indices = { 0,1,1 };

		vertices[0].Position = start;
		vertices[1].Position = endpoint;

		Model model;

		UnLitMaterial* material = new UnLitMaterial();

		model.meshes.push_back(std::make_shared<Mesh>(vertices, indices, material));

		model.DrawSolidColor(color, true);
}

void GraphicsRender::ClearModelList()
{
	modelAndShaderList.clear();
	transparentmodelAndShaderList.clear();
}


void GraphicsRender::SetSelectedModel(Model* model)

{
	/*if (model->tag == "Particles")
	{
		selectedModel = nullptr;
		return ;
	}*/
	selectedModel = model;
}

Model* GraphicsRender::GetSelectedModel()
{
	return selectedModel;
}

void GraphicsRender::CleanUpData()
{

	for (ModelAndShader* modelAndShader : modelAndShaderList)
	{
		modelAndShader->model = nullptr;
		modelAndShader->shader = nullptr;
		delete modelAndShader->model;

		delete modelAndShader->shader;
		delete modelAndShader;
	}

	modelAndShaderList.clear();

	for (ModelAndShader* modelAndShader : transparentmodelAndShaderList)
	{
		modelAndShader->model = nullptr;
		modelAndShader->shader = nullptr;
		delete modelAndShader->model;
		delete modelAndShader->shader;
		delete modelAndShader;
	}

	transparentmodelAndShaderList.clear();

}
