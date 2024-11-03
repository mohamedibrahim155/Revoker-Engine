#include "FogSystem.h"
#include "ImGui/ImGuiUtils.h"

FogSystem::FogSystem()
{
	name = "Fog System";
	
	InitializeEntity(this);
}



void FogSystem::DrawProperties()
{
	float width = 150;
	Entity::DrawProperties();

	if (!ImGui::TreeNodeEx("Fog Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	DrawDragFloatImGui("Fog Density", fogDensity,0.1f,0,100);
	DrawDragFloatImGui("Fog Start", fogStart, 0.1f, 0, 100);
	DrawDragFloatImGui("Fog End", fogEnd, 0.1f, 0, 100);
	DrawTransformVector3ImGui("Fog End", fogColor,0.59, width);
	DrawBoolImGui("Fog Active", fogActive);
	ImGui::TreePop();
}

void FogSystem::SceneDraw()
{
	Entity::SceneDraw();
}
