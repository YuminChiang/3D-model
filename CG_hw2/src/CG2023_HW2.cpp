#include <Windows.h>

#include "headers.h"
#include "trianglemesh.h"
#include "camera.h"
#include "shaderprog.h"
#include "light.h"


// Global variables.
int screenWidth = 600;
int screenHeight = 600;
char currDir[MAX_PATH] = "";
std::string parentDir = "";
std::string testModelsDir = "";
bool init = false;
// Triangle mesh.
TriangleMesh* mesh = nullptr;
// Lights.
DirectionalLight* dirLight = nullptr;
PointLight* pointLight = nullptr;
SpotLight* spotLight = nullptr;
// Parameters: PLEASE DO NOT CHANGE THESE PARAMETERS.
glm::vec3 dirLightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.6f, 0.6f, 0.6f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.0f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(0.5f, 0.1f, 0.1f);
glm::vec3 spotLightPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightIntensity = glm::vec3(0.5f, 0.5f, 0.1f);
float spotLightCutoffStartInDegree = 30.0f;
float spotLightTotalWidthInDegree = 45.0f;
glm::vec3 ambientLight = glm::vec3(0.2f, 0.2f, 0.2f);
// Camera.
Camera* camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;
// Shaders.
FillColorShaderProg* fillColorShader = nullptr;
PhongShadingDemoShaderProg* phongShadingShader = nullptr;
// UI.
const float lightMoveSpeed = 0.2f;

// SceneObject.
struct SceneObject
{
    SceneObject() {
        mesh = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
    }
    TriangleMesh* mesh;
    glm::mat4x4 worldMatrix;
};
SceneObject sceneObj;

// ScenePointLight (for visualization of a point light).
struct ScenePointLight
{
    ScenePointLight() {
        light = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
        visColor = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    PointLight* light;
    glm::mat4x4 worldMatrix;
    glm::vec3 visColor;
};
ScenePointLight pointLightObj;
ScenePointLight spotLightObj;

// Function prototypes.
void ReleaseResources();
void ResetLights();
void CreateLights();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
void SetupRenderState();
void LoadObjects(const std::string&);
void CreateCamera();
void CreateShaderLib();


void ReleaseResources()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Delete scene objects and lights.
    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;
    }
    if (pointLight != nullptr) {
        delete pointLight;
        pointLight = nullptr;
    }
    if (spotLight != nullptr) {
        delete spotLight;
        spotLight = nullptr;
    }
    if (dirLight != nullptr) {
        delete dirLight;
        dirLight = nullptr;
    }
    // Delete camera.
    if (camera != nullptr) {
        delete camera;
        camera = nullptr;
    }
    // Delete shaders.
    if (fillColorShader != nullptr) {
        delete fillColorShader;
        fillColorShader = nullptr;
    }
    if (phongShadingShader != nullptr) {
        delete phongShadingShader;
        phongShadingShader = nullptr;
    }
}

static float curRotationY = 0.0f;
const float rotStep = 0.025f;
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render a triangle mesh with Phong shading. ------------------------------------------------
    TriangleMesh* mesh = sceneObj.mesh;
    if (sceneObj.mesh != nullptr) {
        // Update transform.
        // curRotationY += rotStep;
        glm::mat4x4 S = glm::scale(glm::mat4x4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        glm::mat4x4 R = glm::rotate(glm::mat4x4(1.0f), glm::radians(curRotationY), glm::vec3(0, 1, 0));
        sceneObj.worldMatrix = S * R;
        // -------------------------------------------------------
		// Note: if you want to compute lighting in the View Space, 
        //       you might need to change the code below.
		// -------------------------------------------------------
        // normal_world = inversr(M)^T * n 在 world space 算 lighting
        // 所以 position 和 normal 都要 transform 到 world space
        glm::mat4x4 normalMatrix = glm::transpose(glm::inverse(sceneObj.worldMatrix));
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * sceneObj.worldMatrix;
        
        // -------------------------------------------------------
		// Add your rendering code here.
		// -------------------------------------------------------
        phongShadingShader->Bind();
        // Transformation matrix.
        glUniformMatrix4fv(phongShadingShader->GetLocM(), 1, GL_FALSE, glm::value_ptr(sceneObj.worldMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocNM(), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(phongShadingShader->GetLocCameraPos(), 1,  glm::value_ptr(camera->GetCameraPos()));
        
        for (auto&& subMesh : sceneObj.mesh->GetsubMeshes()) {
            // Material properties.
            glUniform3fv(phongShadingShader->GetLocKa(), 1, glm::value_ptr(subMesh.material->GetKa()));
            glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(subMesh.material->GetKd()));
            glUniform3fv(phongShadingShader->GetLocKs(), 1, glm::value_ptr(subMesh.material->GetKs()));
            glUniform1f(phongShadingShader->GetLocNs(), subMesh.material->GetNs());        
            // Light data.
            if (dirLight != nullptr) {
                glUniform3fv(phongShadingShader->GetLocDirLightDir(), 1, glm::value_ptr(dirLight->GetDirection()));
                glUniform3fv(phongShadingShader->GetLocDirLightRadiance(), 1, glm::value_ptr(dirLight->GetRadiance()));
            }
            if (pointLight != nullptr) {
                glUniform3fv(phongShadingShader->GetLocPointLightPos(), 1, glm::value_ptr(pointLight->GetPosition()));
                glUniform3fv(phongShadingShader->GetLocPointLightIntensity(), 1, glm::value_ptr(pointLight->GetIntensity()));
            }
            if (spotLight != nullptr) {
                glUniform3fv(phongShadingShader->GetLocSpotLightPos(), 1, glm::value_ptr(spotLight->GetPosition()));
                glUniform3fv(phongShadingShader->GetLocSpotLightIntensity(), 1, glm::value_ptr(spotLight->GetIntensity()));
                glUniform3fv(phongShadingShader->GetLocSpotLightDir(), 1, glm::value_ptr(spotLight->GetDirection()));
                glUniform1f(phongShadingShader->GetLocSpotLightCutoffDeg(), spotLight->GetCutoffDeg());
                glUniform1f(phongShadingShader->GetLocSpotLightTotalWidthDeg(), spotLight->GetTotalWidthDeg());
            }

            glUniform3fv(phongShadingShader->GetLocAmbientLight(), 1, glm::value_ptr(ambientLight));

            // Render the submesh.
            sceneObj.mesh->RenderSubMesh(subMesh);
        } 
        // Render.
        //sceneObj.mesh->Render();
        phongShadingShader->UnBind();
    }
    
    // Visualize the light with fill color. ------------------------------------------------------
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    PointLight* pointLight = pointLightObj.light;
    if (pointLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), pointLight->GetPosition());
        pointLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * pointLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(pointLightObj.visColor));
        // Render the point light.
        pointLight->Draw();
        fillColorShader->UnBind();
    }
    SpotLight* spotLight = (SpotLight*)(spotLightObj.light);
    if (spotLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), spotLight->GetPosition());
        spotLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * spotLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(spotLightObj.visColor));
        // Render the point light.
        spotLight->Draw();
        fillColorShader->UnBind();
    }
    // -------------------------------------------------------------------------------------------

    glutSwapBuffers();
}

void ReshapeCB(int w, int h)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Update viewport.
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, screenWidth, screenHeight);
    // Adjust camera and projection.
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void ProcessSpecialKeysCB(int key, int x, int y)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc. 
    switch (key) {
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    
    // Point light control.
    case GLUT_KEY_LEFT:
        if (pointLight != nullptr)
            pointLight->MoveLeft(lightMoveSpeed);
        break;
    case GLUT_KEY_RIGHT:
        if (pointLight != nullptr)
            pointLight->MoveRight(lightMoveSpeed);
        break;
    case GLUT_KEY_UP:
        if (pointLight != nullptr)
            pointLight->MoveUp(lightMoveSpeed);
        break;
    case GLUT_KEY_DOWN:
        if (pointLight != nullptr)
            pointLight->MoveDown(lightMoveSpeed);
        break;

    default:
        break;
    }
}

void ProcessKeysCB(unsigned char key, int x, int y)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // Handle other keyboard inputs those are not defined as special keys.
    if (key == 27) {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }
    // Spot light control.
    if (spotLight != nullptr) {
        if (key == 'a')
            spotLight->MoveLeft(lightMoveSpeed);
        if (key == 'd')
            spotLight->MoveRight(lightMoveSpeed);
        if (key == 'w')
            spotLight->MoveUp(lightMoveSpeed);
        if (key == 's')
            spotLight->MoveDown(lightMoveSpeed);
    }
}

void SelectFileCallback(int selection) 
{
    if (selection == 0) {
        OPENFILENAMEA ofn;
        char selectedFilePath[MAX_PATH] = "";
        
        std::cout << "Current Directory: " << currDir << std::endl;
        std::cout << "TestModels_HW2 Directory: " << testModelsDir << std::endl;

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lpstrInitialDir = testModelsDir.c_str();
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = "OBJ Files\0*.obj\0All Files\0*.*\0";
        ofn.lpstrFile = selectedFilePath;
        ofn.nMaxFile = sizeof(selectedFilePath);
        ofn.lpstrTitle = "Select OBJ File";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameA(&ofn)) {
            // Get selected OBJ file name without extension
            std::filesystem::path filePath(selectedFilePath);
            std::string objName = filePath.stem().string();
            std::cout << "Selected OBJ File: " << objName << std::endl;
            
            // Replace backslashes with forward slashes in the path
            std::replace(std::begin(selectedFilePath), std::end(selectedFilePath), '\\', '/');

            // Load the selected OBJ file
            LoadObjects(selectedFilePath);
        }
        else {
            std::wcerr << L"File selection cancelled or an error occurred." << std::endl;
        }
    }
}

void ProcessMouseClickCB(int button, int state, int x, int y) 
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (!init) {
            // Get current directory
            GetCurrentDirectoryA(MAX_PATH, currDir);
            std::filesystem::path currPath(currDir);
            // Set parent path
            // std::filesystem::path parentPath = currPath.parent_path();
            // parentDir = parentPath.string();
            // Set TestModels_HW2 directory
            testModelsDir = (std::filesystem::path(currDir) / "TestModels_HW2").string();
            init = true;
        }
        int menu = glutCreateMenu(SelectFileCallback);
        glutAddMenuEntry("Load 3D Model", 0);
        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }
}

void SetupRenderState()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r), 
        (GLclampf)(clearColor.g), 
        (GLclampf)(clearColor.b), 
        (GLclampf)(clearColor.a)
    );
}

void LoadObjects(const std::string& modelPath)
{
    // -------------------------------------------------------
	// Note: you can change the code below if you want to load
    //       the model dynamically.
	// -------------------------------------------------------

    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;
        ResetLights();
        CreateLights();
    }
    mesh = new TriangleMesh();
    mesh->LoadFromFile(modelPath, true);
    mesh->ShowInfo();
    // Create and upload vertex/index buffers.
    mesh->CreateBuffers();
    sceneObj.mesh = mesh;
}

void ResetLights() 
{
    delete pointLight;
    delete spotLight;
    delete dirLight;
    pointLight = nullptr;
    spotLight = nullptr;
    dirLight = nullptr;
}

void CreateLights()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // Create a directional light.
    dirLight = new DirectionalLight(dirLightDirection, dirLightRadiance);
    // Create a point light.
    pointLight = new PointLight(pointLightPosition, pointLightIntensity);
    pointLightObj.light = pointLight;
    pointLightObj.visColor = glm::normalize((pointLightObj.light)->GetIntensity());
    // Create a spot light.
    spotLight = new SpotLight(spotLightPosition, spotLightIntensity, spotLightDirection, 
    spotLightCutoffStartInDegree, spotLightTotalWidthInDegree);
    spotLightObj.light = spotLight;
    spotLightObj.visColor = glm::normalize((spotLightObj.light)->GetIntensity());
}

void CreateCamera()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    
    // Create a camera and update view and proj matrices.
    camera = new Camera((float)screenWidth / (float)screenHeight);
    camera->UpdateView(cameraPos, cameraTarget, cameraUp);
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateShaderLib()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    fillColorShader = new FillColorShaderProg();
    if (!fillColorShader->LoadFromFiles("shaders/fixed_color.vs", "shaders/fixed_color.fs"))
        exit(1);

    phongShadingShader = new PhongShadingDemoShaderProg();
    if (!phongShadingShader->LoadFromFiles("shaders/phong_shading_demo.vs", "shaders/phong_shading_demo.fs"))
        exit(1);
}

int main(int argc, char** argv)
{
    // Setting window properties.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("HW2: Lighting and Shading");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "GLEW initialization error: " 
                  << glewGetErrorString(res) << std::endl;
        return 1;
    }

    // Initialization.
    SetupRenderState();
    // LoadObjects(""TODO: ADD FILE PATH");
    CreateLights();
    CreateCamera();
    CreateShaderLib();

    // Register callback functions.
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutSpecialFunc(ProcessSpecialKeysCB);
    glutKeyboardFunc(ProcessKeysCB);
    // 按右鍵後才 LoadObjects
    glutMouseFunc(ProcessMouseClickCB);

    // Start rendering loop.
    glutMainLoop();

    return 0;
}
