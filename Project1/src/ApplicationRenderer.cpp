#include"ApplicationRenderer.h"

#include "SceneManager/Scenes/SceneOne.h"
#include "SceneManager/Scenes/SceneTwo.h"
#include "SceneManager/Scenes/SceneThree.h"
#include "SceneManager/Scenes/SceneFour.h"
#include "SceneManager/Scenes/SceneFive.h"
#include "SceneManager/Scenes/SceneSix.h"
#include "SceneManager/Scenes/SceneSeven.h"
#include "SceneManager/Scenes/SceneEight.h"

ApplicationRenderer::ApplicationRenderer()
{
    sceneViewcamera = new Camera();
    sceneViewcamera->name = "Sceneview Camera";
    CameraManager::GetInstance().sceneViewCamera = sceneViewcamera;

    gameScenecamera = new Camera();
    gameScenecamera->name = "GameScene Camera";

    renderTextureCamera = new Camera();
    renderTextureCamera->name = "RenderTexture Camera";
}

ApplicationRenderer::~ApplicationRenderer()
{
}



void ApplicationRenderer::WindowInitialize(int width, int height,  std::string windowName)
{
    windowWidth = width;
    WindowHeight = height;
    lastX = windowWidth / 2.0f;
    lastY= WindowHeight / 2.0f;

    glfwInit();



    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int x, int y)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(w))->SetViewPort(w, x, y);
        });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->KeyCallBack(window, key, scancode, action, mods);
        });


    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xposIn, double yposIn)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->MouseCallBack(window, xposIn, yposIn);
        });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->MouseHeldCallBack(window, button, action, mods);
        });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            static_cast<ApplicationRenderer*>(glfwGetWindowUserPointer(window))->MouseScroll(window, xoffset, yoffset);
        });
  
    glfwSetJoystickCallback([](int jid, int event)
        {
            static_cast<ApplicationRenderer*>(glfwGetJoystickUserPointer(jid))->Joystick_callback(jid, event);
        }
    );
   


   

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 2.0f;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
   // ImGui_ImplOpenGL3_Init("#version 130");

    //Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Query and print OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    if (version) {
        std::cout << "OpenGL Version: " << version << std::endl;
    }
    else 
    {
        std::cerr << "Failed to retrieve OpenGL version\n";
     
    }


    FrameBufferSpecification specification;

    specification.width = windowWidth;
    specification.height = WindowHeight;
    specification.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
    

    sceneViewframeBuffer = new FrameBuffer(specification);

    gameframeBuffer = new FrameBuffer(specification);

    EditorLayout::GetInstance().applicationRenderer = this;
  

    InitializeShaders();
   
    GraphicsRender::GetInstance().InitializeGraphics();

    DebugModels::GetInstance().defaultCube = new Model("Models/DefaultCube/DefaultCube.fbx", false, true);
    DebugModels::GetInstance().defaultSphere = new Model("Models/DefaultSphere/DefaultSphere.fbx", false, true);
    DebugModels::GetInstance().defaultQuad = new Model("Models/DefaultQuad/DefaultQuad.fbx", false, true);

    InitializeSkybox();


    sceneViewcamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 1000.0f);
    sceneViewcamera->transform.position = glm::vec3(-79.46, 11.76, 107.19);
    sceneViewcamera->transform.SetRotation(glm::vec3(-41.70, -31.50,0));

    gameScenecamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 1000.0f);
    gameScenecamera->transform.position = glm::vec3(-85.88, 9.03, 115.62);
    gameScenecamera->transform.SetRotation(glm::vec3(-6.10, -20.50,0));

    renderTextureCamera->InitializeCamera(CameraType::PERSPECTIVE, 45.0f, 0.1f, 1000.0f);
    renderTextureCamera->transform.position = glm::vec3(0, 0, -1.0f);

    renderTextureCamera->IntializeRenderTexture(specification);
  
    sceneViewcamera->postprocessing->InitializePostProcessing();
    gameScenecamera->postprocessing->InitializePostProcessing();

    isImguiPanelsEnable = true;

    SceneManager::GetInstance().application = this;

    PhysXEngine::GetInstance().InitializePhysX();
}

void ApplicationRenderer::InitializeShaders()
{
    defaultShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag");
   
    solidColorShader = new Shader("Shaders/SolidColor_Vertex.vert", "Shaders/SolidColor_Fragment.frag", SOLID);
    stencilShader = new Shader("Shaders/StencilOutline.vert", "Shaders/StencilOutline.frag", OPAQUE);
    //ScrollShader = new Shader("Shaders/ScrollTexture.vert", "Shaders/ScrollTexture.frag");

    alphaBlendShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag", ALPHA_BLEND);
    alphaBlendShader->blendMode = ALPHA_BLEND;

    alphaCutoutShader = new Shader("Shaders/DefaultShader_Vertex.vert", "Shaders/DefaultShader_Fragment.frag", ALPHA_CUTOUT);
    alphaCutoutShader->blendMode = ALPHA_CUTOUT;

    skyboxShader = new Shader("Shaders/SkyboxShader.vert", "Shaders/SkyboxShader.frag");
    skyboxShader->modelUniform = false;

    boneAnimationShader = new Shader("Shaders/AnimationShader.vert", "Shaders/AnimationShader.frag");

    defaultInstanceShader = new Shader("Shaders/MeshInstanceShader.vert", "Shaders/MeshInstanceShader.frag");
    defaultInstanceShader->blendMode = OPAQUE;
    defaultInstanceShader->modelUniform = false;

    grassInstanceShader = new Shader("Shaders/Grass/GrassMeshInstanceShader.vert", "Shaders/Grass/GrassMeshInstanceShader.frag", ALPHA_CUTOUT);
    grassInstanceShader->modelUniform = false;

    particleShader = new Shader("Shaders/ParticleShader.vert", "Shaders/ParticleShader.frag");
    particleShader->blendMode = ALPHA_BLEND;

    GraphicsRender::GetInstance().defaultShader = defaultShader;
    GraphicsRender::GetInstance().solidColorShader = solidColorShader;
    GraphicsRender::GetInstance().stencilShader = stencilShader; 
    GraphicsRender::GetInstance().boneAnimationShader = boneAnimationShader;
    GraphicsRender::GetInstance().alphaBlendShader = alphaBlendShader;
    GraphicsRender::GetInstance().alphaCutoutShader = alphaCutoutShader;
    GraphicsRender::GetInstance().defaultInstanceShader = defaultInstanceShader;
    GraphicsRender::GetInstance().grassInstanceShader = grassInstanceShader;
    GraphicsRender::GetInstance().particleShader = particleShader;

    LightManager::GetInstance().AddShader(defaultShader);
    LightManager::GetInstance().AddShader(boneAnimationShader);
    LightManager::GetInstance().AddShader(alphaBlendShader);
    LightManager::GetInstance().AddShader(alphaCutoutShader);
    LightManager::GetInstance().AddShader(defaultInstanceShader);
    LightManager::GetInstance().AddShader(grassInstanceShader);
}

void ApplicationRenderer::InitializeSkybox()
{
    skyBoxModel = new Model("Models/DefaultCube/DefaultCube.fbx", false, true);
    skyBoxModel->meshes[0]->meshMaterial = new SkyboxMaterial();

    skyBoxMaterial = skyBoxModel->meshes[0]->meshMaterial->skyboxMaterial();

    std::vector<std::string> faces
    {
       ("Textures/skybox/right.png"),
       ("Textures/skybox/left.png"),
       ("Textures/skybox/top.png"),
       ("Textures/skybox/bottom.png"),
       ("Textures/skybox/front.png"),
       ("Textures/skybox/back.png")
    };

    skyBoxMaterial->skyBoxTexture->LoadTexture(faces);

    GraphicsRender::GetInstance().SkyBoxModel = skyBoxModel;
}



void ApplicationRenderer::Start()
{

    BaseScene* sceneOne = new SceneOne("PlayerTest");
    BaseScene* sceneTwo = new SceneTwo("MeshInstance");
    BaseScene* sceneThree = new SceneThree("LOD_Test");
    BaseScene* sceneFour = new SceneFour("GrassMeshInstance");
    BaseScene* sceneFive = new SceneFive("MainGame");
    BaseScene* sceneSix = new SceneSix("AI_Scene");
    BaseScene* sceneSeven = new SceneSeven("occlusion");
    BaseScene* sceneEight = new SceneEight("SoftBody");

    SceneManager::GetInstance().OnChangeScene("PlayerTest");

    FPS* fps = new FPS();
    fogSystem = new FogSystem();

}


void ApplicationRenderer::Render()
{
   
    Start();
  
    EditorLayout::GetInstance().InitializeEditors();

    Time::GetInstance().lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        Time::GetInstance().SetCurrentTime(glfwGetTime());

        EngineGameLoop();

        EngineGraphicsRender();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ShutDown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void ApplicationRenderer::EngineGraphicsRender()
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    if (isImguiPanelsEnable)
    {
        PanelManager::GetInstance().Update((float)windowWidth, (float)WindowHeight);
    }

    if (!isMaximizePressed)
    {
        RenderForCamera(sceneViewcamera, sceneViewframeBuffer, true);
    }

    for (Camera* camera :  CameraManager::GetInstance().GetCameras())
    {
        if (camera->renderTexture == nullptr)
        {
            RenderForCamera(camera, gameframeBuffer);
        }
        else
        {
            RenderForCamera(camera, camera->renderTexture->framebuffer); 
        }
       
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void ApplicationRenderer::EngineGameLoop()
{
    MouseInputUpdate();
    JoyStickInputs();
    ProcessInput(window);

    if (isPlayMode)
    {
        EntityManager::GetInstance().Update(Time::GetInstance().deltaTime);
        InputManager::GetInstance().Update(Time::GetInstance().deltaTime);
        SceneManager::GetInstance().Update();
        ParticleSystemManager::GetInstance().Update(Time::GetInstance().deltaTime);
        PhysicsEngine::GetInstance().PhysicsEngineUpdate(Time::GetInstance().deltaTime);
    }

    PostRender();
}
void ApplicationRenderer::RenderForCamera(Camera* camera, FrameBuffer* framebuffer, bool isSceneView)
{

    framebuffer->Bind();

    GraphicsRender::GetInstance().Clear();

    projection = camera->GetProjectionMatrix();

    view = camera->GetViewMatrix();

    skyBoxView = glm::mat4(glm::mat3(camera->GetViewMatrix()));

    LightManager::GetInstance().RenderLights();


    defaultShader->Bind();
    defaultShader->setMat4("projection", projection);
    defaultShader->setMat4("view", view);
    defaultShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    defaultShader->setFloat("time", scrollTime);
    defaultShader->setBool("isDepthBuffer", isDepth);

    defaultShader->setFloat("fogDensity", fogSystem->fogDensity);
    defaultShader->setFloat("fogStart", fogSystem->fogStart);
    defaultShader->setFloat("fogEnd", fogSystem->fogEnd);
    defaultShader->setVec3("fogColor", fogSystem->fogColor);
    defaultShader->setBool("fogActive", fogSystem->fogActive);

    defaultShader->setBool("isCellShading", isCellShade);


    boneAnimationShader->Bind();
    boneAnimationShader->setMat4("projection", projection);
    boneAnimationShader->setMat4("view", view);
    boneAnimationShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    boneAnimationShader->setBool("isDepthBuffer", isDepth);
    boneAnimationShader->setFloat("fogDensity", fogSystem->fogDensity);
    boneAnimationShader->setFloat("fogStart", fogSystem->fogStart);
    boneAnimationShader->setFloat("fogEnd", fogSystem->fogEnd);
    boneAnimationShader->setVec3("fogColor", fogSystem->fogColor);
    boneAnimationShader->setBool("fogActive", fogSystem->fogActive);


    alphaBlendShader->Bind();
    alphaBlendShader->setMat4("projection", projection);
    alphaBlendShader->setMat4("view", view);
    alphaBlendShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    alphaBlendShader->setFloat("time", scrollTime);
    alphaBlendShader->setBool("isDepthBuffer", isDepth);

    alphaCutoutShader->Bind();
    alphaCutoutShader->setMat4("projection", projection);
    alphaCutoutShader->setMat4("view", view);
    alphaCutoutShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    alphaCutoutShader->setFloat("time", scrollTime);
    alphaCutoutShader->setBool("isDepthBuffer", isDepth);

    solidColorShader->Bind();
    solidColorShader->setMat4("projection", projection);
    solidColorShader->setMat4("view", view);

    stencilShader->Bind();
    stencilShader->setMat4("projection", projection);
    stencilShader->setMat4("view", view);

  

    defaultInstanceShader->Bind();
    defaultInstanceShader->setMat4("projection", projection);
    defaultInstanceShader->setMat4("view", view);
    defaultInstanceShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    defaultInstanceShader->setFloat("time", scrollTime);
    defaultInstanceShader->setBool("isDepthBuffer", false);
    defaultInstanceShader->setFloat("fogDensity", fogSystem->fogDensity);
    defaultInstanceShader->setFloat("fogStart", fogSystem->fogStart);
    defaultInstanceShader->setFloat("fogEnd", fogSystem->fogEnd);
    defaultInstanceShader->setVec3("fogColor", fogSystem->fogColor);
    defaultInstanceShader->setBool("fogActive", fogSystem->fogActive);


    grassInstanceShader->Bind();
    grassInstanceShader->setMat4("projection", projection);
    grassInstanceShader->setMat4("view", view);
    grassInstanceShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    grassInstanceShader->setFloat("fogDensity", fogSystem->fogDensity);
    grassInstanceShader->setFloat("fogStart", fogSystem->fogStart);
    grassInstanceShader->setFloat("fogEnd", fogSystem->fogEnd);
    grassInstanceShader->setVec3("fogColor", fogSystem->fogColor);
    grassInstanceShader->setBool("fogActive", fogSystem->fogActive);

    particleShader->Bind();
    particleShader->setMat4("projection", projection);
    particleShader->setMat4("view", view);

    glDepthFunc(GL_LEQUAL);
    skyboxShader->Bind();
    skyboxShader->setMat4("projection", projection);
    skyboxShader->setMat4("view", skyBoxView);
    skyboxShader->setVec3("viewPos", camera->transform.position.x, camera->transform.position.y, camera->transform.position.z);
    skyboxShader->setFloat("fogDensity", fogSystem->fogDensity);
    skyboxShader->setFloat("fogStart", fogSystem->fogStart);
    skyboxShader->setFloat("fogEnd", fogSystem->fogEnd);
    skyboxShader->setVec3("fogColor", fogSystem->fogColor);
    skyboxShader->setBool("fogActive", fogSystem->fogActive);

    GraphicsRender::GetInstance().SkyBoxModel->Draw(skyboxShader);
    glDepthFunc(GL_LESS);

    if (isSceneView)
    {
        EntityManager::GetInstance().Render();
        SceneManager::GetInstance().Render();
    }
    GraphicsRender::GetInstance().Draw(isSceneView);
    GraphicsRender::GetInstance().SetCamera(camera);

    ParticleSystemManager::GetInstance().Render();

    if (camera->isPostprocessing)
    {
        camera->postprocessing->ApplyPostprocessing(framebuffer);
    }

     framebuffer->Unbind();
}

void ApplicationRenderer::ChangeCursorState(eCursorState state)
{
    currentCursorState = state;
    switch (currentCursorState)
    {
    case eCursorState::VISIBLE:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case eCursorState::HIDDEN:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case eCursorState::LOCKED:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    }
}

void ApplicationRenderer::PostRender()
{
    if (isPlayMode)
    {
        PhysXEngine::GetInstance().InitializePhysXObjects();
        PhysXEngine::GetInstance().Update(Time::GetInstance().deltaTime);
    }

    OcculsionManager::GetInstance().UpdateOcculsion();
}

void ApplicationRenderer::Clear()
{
    GLCALL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
   //glStencilMask(0x00);
}

void ApplicationRenderer::ShutDown()
{

    PhysXEngine::GetInstance().ShutDown();
    SceneManager::GetInstance().ShutDown();
}

void ApplicationRenderer::ProcessInput(GLFWwindow* window)
{

    float cameraSpeed=25;

    if (EditorLayout::GetInstance().IsViewportHovered())
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(FORWARD, Time::GetInstance().deltaTime * cameraSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(BACKWARD, Time::GetInstance().deltaTime * cameraSpeed);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(LEFT, Time::GetInstance().deltaTime * cameraSpeed);

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            sceneViewcamera->ProcessKeyboard(RIGHT, Time::GetInstance().deltaTime * cameraSpeed);

        }
    }

}


 void ApplicationRenderer::SetViewPort(GLFWwindow* window, int width, int height)
{
 //   glViewport(0, 0, width, height);
}

 void ApplicationRenderer::KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
 {  
     
         if (action == GLFW_PRESS)
         {
             if (key == GLFW_KEY_ESCAPE)
             {
                 ChangeCursorState(eCursorState::VISIBLE);
             }
             if (key == GLFW_KEY_K)
             {
                 ChangeCursorState(eCursorState::LOCKED);
             }
             
             InputManager::GetInstance().OnKeyPressed(key);
         }
         else if(action == GLFW_RELEASE)
         {
             InputManager::GetInstance().OnKeyReleased(key);
         }
         else if (action == GLFW_REPEAT)
         {
            // InputManager::GetInstance().OnkeyHold(key);
         }

         if (key == GLFW_KEY_P && action == GLFW_PRESS)
         {
             fogSystem->fogActive = !fogSystem->fogActive;
         }
         if (key == GLFW_KEY_L && action == GLFW_PRESS)
         {
             isCellShade = !isCellShade;
         }
         if (key == GLFW_KEY_O && action == GLFW_PRESS)
         {
             GraphicsRender::GetInstance().isDebug = !GraphicsRender::GetInstance().isDebug;
         }
     
 }

 void ApplicationRenderer::MouseCallBack(GLFWwindow* window, double xposIn, double yposIn)
 {

        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
     
      
        currentMousePos.x = xpos;
        currentMousePos.y = ypos;

        mouseDeltaPos = currentMousePos - lastMousePos;
        mouseDeltaPos.y = -mouseDeltaPos.y;

        InputManager::GetInstance().mouseCurrentPosition = currentMousePos;
     
     
        lastMousePos.x = xpos;
        lastMousePos.y = ypos;

         if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && EditorLayout::GetInstance().IsViewportHovered())
         {
            sceneViewcamera->ProcessMouseMovement(mouseDeltaPos.x, mouseDeltaPos.y);
         }

 }

 void ApplicationRenderer::MouseHeldCallBack(GLFWwindow* window, int& button, int& action, int& mods)
 {
     if (action == GLFW_PRESS)
     {
         InputManager::GetInstance().OnMouseButtonPressed(button);
     }
     else if (action == GLFW_RELEASE)
     {
         InputManager::GetInstance().OnMouseButtonReleased(button);
     }
 }

 void ApplicationRenderer::MouseScroll(GLFWwindow* window, double xoffset, double yoffset)
 {
     sceneViewcamera->ProcessMouseScroll(static_cast<float>(yoffset));
 }

 void ApplicationRenderer::Joystick_callback(int jid, int event)
 {
     if (event == GLFW_CONNECTED)
     {
         // The joystick was connected
         printf("JoyStick Connected \n");
         InputManager::GetInstance().SetJoystickConnected(true);
     }
     else if (event == GLFW_DISCONNECTED)
     {
         printf("JoyStick Disconnected \n");
         InputManager::GetInstance().SetJoystickConnected(false);
     }

 }

 void ApplicationRenderer::JoyStickInputs()
 {

     InputManager::GetInstance().SetJoystickConnected(glfwJoystickPresent(GLFW_JOYSTICK_1));

     if (!InputManager::GetInstance().IsJoyStickConnected()) return;
  
     GLFWgamepadstate state;
     if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
     {
         const unsigned int GAMEPAD_BUTTONS = 15;

         for (unsigned int i = 0; i < GAMEPAD_BUTTONS; i++)
         {
             bool isPressed = state.buttons[i] == GLFW_PRESS;

             if (isPressed)
             {
                 InputManager::GetInstance().OnJoystickButtonPressed((eJoystickButton)i);

                 InputManager::GetInstance().OnJoystickButtonHold((eJoystickButton)i);
             }
             else
             {
                 InputManager::GetInstance().OnJoystickButtonReleased((eJoystickButton)i);
             }
         }

         InputManager::GetInstance().SetJoysitckLeftAxis(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
         InputManager::GetInstance().SetJoysitckRightAxis(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
         InputManager::GetInstance().SetJoysitckTrigger(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER],state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
     }
     


 }

 void ApplicationRenderer::MouseInputUpdate()
 {

      // Calculate the raw mouse delta
     glm::vec2 rawMouseDelta = InputManager::GetInstance().mouseCurrentPosition - InputManager::GetInstance().mouseLastPosition;

     // Invert Y-axis
     rawMouseDelta.y = -rawMouseDelta.y;

     // Apply smoothing (exponential smoothing)
  
     glm::vec2 smoothedMouseDelta = InputManager::GetInstance().smoothedMouseDelta * (1.0f - mouseSmoothingFactor) + rawMouseDelta * mouseSmoothingFactor;
     InputManager::GetInstance().smoothedMouseDelta = smoothedMouseDelta;

     InputManager::GetInstance().SetMouseSmoothDelta(smoothedMouseDelta);

     // Update the last mouse position for the next frame
     InputManager::GetInstance().mouseLastPosition = InputManager::GetInstance().mouseCurrentPosition;
 }