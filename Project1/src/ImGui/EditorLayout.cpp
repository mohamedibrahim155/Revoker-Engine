#include "EditorLayout.h"
#include "../ApplicationRenderer.h"

EditorLayout& EditorLayout::GetInstance()
{
    static EditorLayout editorInstance;

    return editorInstance;
}

Object* EditorLayout::GetCurrentSelectedObject()
{
    return currentSelectedObject;
}



void EditorLayout::InitializeEditors()
{
    windowDockingPanel = new WindowDockingPanel();
    hierarchyPanel = new Heirachy();
    inspectorPanel = new Inspector();
    sceneViewportPanel = new SceneViewportPanel();
    gameviewportPanel = new GameViewportPanel();
    toolPanel = new ToolPanel();

    sceneViewportPanel->frameBuffer = applicationRenderer->sceneViewframeBuffer;
    sceneViewportPanel->sceneViewportCamera = applicationRenderer->sceneViewcamera;
  
    toolPanel->isApplicationPlayMode = &applicationRenderer->isPlayMode;

    gameviewportPanel->frameBuffer = applicationRenderer->gameframeBuffer;
    gameviewportPanel->gameViewportResolution = ImVec2(1920, 1080);
}

void EditorLayout::SetSelectedObjects(std::vector<Object*> objects)
{
    if (!selectedObjectsList.empty())
    {
        SetSelectedObjectsEnable(false);
    }
        
    selectedObjectsList.clear();

    selectedObjectsList = objects;

    SetSelectedObjectsEnable(true);

}

void EditorLayout::SetSelectedObjectsEnable(bool isEnable)
{
    for (Object* object:  selectedObjectsList)
    {
        object->isSelected = isEnable;
    }
}

void EditorLayout::SetIsMaxiizePressedState(bool state)
{
    applicationRenderer->isMaximizePressed = state;
}

bool EditorLayout::IsViewportHovered()
{
    return sceneViewportPanel->isHovered;
}

bool EditorLayout::IsGameViewportHovered()
{
    return gameviewportPanel->isHovered;
}

bool EditorLayout::IsMaximizePressed()
{
    return applicationRenderer->isMaximizePressed;
}

std::vector<Object*> EditorLayout::GetSelectedObjects()
{
    return selectedObjectsList;
}
