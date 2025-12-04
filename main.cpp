#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "mesh.h"
#include "model.h"
#include "shader_s.h"
#include "filesystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const unsigned int IMGUI_WINDOW_WIDTH = 180;
const unsigned int IMGUI_WINDOW_HEIGHT = 650;

// light
const glm::vec3 LIGHT_POSITION = glm::vec3(-1.0f, 1.0f, 1.0f);
const glm::vec3 LIGHT_SCALE = glm::vec3(.25f);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

float radius = 4.0f; // rotation radius (adjustable via slider)
float rotationRate = 0.035f; // radians per second (adjustable via slider)
float cameraPosition = 0.0f; // camera position around origin (0-1)
bool autoSpin = true; // toggle for automatic camera rotation

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ShaderDemos", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    //ImGui::StyleColorsLight();

    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    // build and compile shaders
    // -------------------------

    // load textures
    // -------------
    unsigned int floorTexture = loadTexture("resources/container.jpg");
    Shader* floorShader = new Shader("shaders/model/floor.v", "shaders/model/blinnPhong.f");
    floorShader->use();
    floorShader->setBool("useTexture", true);
    floorShader->setInt("texture_diffuse1", 0);

    // Model shader selection system
    const char* modelShaderNames[] = { "Blinn-Phong", "Fresnel", "OS Normals", "Cell Shaded" };
    const char* modelShaderPaths[] = {
        "Shaders/model/blinnPhong.f",
        "Shaders/model/fresnel.f",
        "Shaders/model/normals.f",
        "Shaders/model/cellShading.f"
    };
    int currentModelShaderIndex = 0;
    Shader* modelShader = new Shader("Shaders/model/model.v", modelShaderPaths[currentModelShaderIndex]);

    // Light sphere shader
    Shader lightShader("shaders/model/model.v", "shaders/model/light.f");

    // Model local color (adjustable via color picker)
    float modelLocalColor[3] = { 0.82f, 0.09f, 0.09f }; // RGB color

    // Light color (adjustable via color picker)
    float lightColor[3] = { 1.0f, 1.0f, 1.0f }; // RGB color

    // Point light constants (passed to all model shaders)
    glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 lightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;

    // Model selection
    const char* modelNames[] = { "Suzanne", "Utah Teapot", "Torus"};
    const char* modelPaths[] = {
        "resources/suzanne/suzanne.obj",
        "resources/teapot.obj",
        "resources/coffee_cup.obj"
    };
    int currentModelIndex = 0; // Start with Suzanne (index 0)
    // Load model
    Model* ourModel = new Model(modelPaths[currentModelIndex]);

    // Load light sphere model
    Model* lightModel = new Model("resources/sphere.obj");

    // Post-processing shader selection system
    const char* shaderNames[] = { "None", "Invert", "Dithering", "Gaussian Blur", "Kuwahara", "Sharpen", "Sobel", "Worley" };
    const char* shaderPaths[] = {
        "shaders/postProcessing/ppDefault.f",
        "shaders/postProcessing/ppInvert.f",
        "shaders/postProcessing/ppDithering.f",
        "shaders/postProcessing/ppGaussian.f",
        "shaders/postProcessing/ppKuwahara.f",
        "shaders/postProcessing/ppSharpen.f",
        "shaders/postProcessing/ppSobel.f",
        "shaders/postProcessing/ppWorley.f",
    };
    int currentShaderIndex = 0;
    Shader* screenShader = new Shader("shaders/postProcessing/screen.v", shaderPaths[currentShaderIndex]);

#pragma region data
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float planeVertices[] = {
        // positions          // normals           // texture Coords
         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 2.0f
    };
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
#pragma endregion
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // shader configuration
    // --------------------
    screenShader->use();
    screenShader->setInt("screenTexture", 0);

    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw as wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // start camera per frame logic
        float angle;
        if (autoSpin)
        {
            // Auto-spin mode: use time-based rotation
            angle = currentFrame * rotationRate;
            cameraPosition = fmod(angle / (2.0f * 3.14159265f), 1.0f); // Update slider to match
        }
        else
        {
            // Manual mode: use slider position
            angle = cameraPosition * 2.0f * 3.14159265f; // Convert 0-1 to 0-2π
        }

        float camX = sin(angle) * radius;
        float camZ = cos(angle) * radius;

        // 1. Set the orbiting position
        camera.Position = glm::vec3(camX, 1.0f, camZ);

        // 2. Force the camera to look at the origin (0, 0, 0)
        // This calls the new LookAtTarget method which updates Yaw, Pitch, and Front vector.
        camera.LookAtTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        // render
        // ------
        // bind to framebuffer and draw scene as we normally would to color texture 
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

        // make sure we clear the framebuffer's content
        glClearColor(0.13f, 0.13f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup common view and projection matrices
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        modelShader->use();
        // Pass point light uniforms (OpenGL ignores uniforms not used by the shader)
        modelShader->setVec3("light.position", LIGHT_POSITION);
        modelShader->setVec3("light.ambient", lightAmbient);
        modelShader->setVec3("light.diffuse", lightDiffuse);
        modelShader->setVec3("light.specular", lightSpecular);
        modelShader->setFloat("light.constant", lightConstant);
        modelShader->setFloat("light.linear", lightLinear);
        modelShader->setFloat("light.quadratic", lightQuadratic);

        // Pass model local color and light color
        modelShader->setVec3("localColor", glm::vec3(modelLocalColor[0], modelLocalColor[1], modelLocalColor[2]));
        modelShader->setVec3("lightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
        modelShader->setVec3("viewPos", camera.Position);
        modelShader->setBool("useTexture", false); // Set to true if you want to use textures

        // set matrix uniforms for model
        modelShader->setMat4("projection", projection);
        modelShader->setMat4("view", view);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.2f, 0.2f, 0.25f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f, 0.7f, 0.7f));
        modelShader->setMat4("model", modelMatrix);
        ourModel->Draw(*modelShader);

        // Second model - at an angle
        glm::mat4 modelMatrix2 = glm::mat4(1.0f);
        modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-0.8f, 0.3f, 0.5f));
        modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(-75.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(0.4f));
        modelShader->setMat4("model", modelMatrix2);
        ourModel->Draw(*modelShader);

        // Light sphere
        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        lightShader.setVec3("lightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
        glm::mat4 lightModelMat = glm::mat4(1.0f);
        lightModelMat = glm::translate(lightModelMat, LIGHT_POSITION);
        lightModelMat = glm::scale(lightModelMat, LIGHT_SCALE);
        lightShader.setMat4("model", lightModelMat);
        lightModel->Draw(lightShader);

        // floor using floorShader with texture
        floorShader->use();
        floorShader->setMat4("view", view);
        floorShader->setMat4("projection", projection);
        floorShader->setMat4("model", glm::mat4(1.0f));
        floorShader->setVec3("viewPos", camera.Position);
        floorShader->setVec3("light.position", LIGHT_POSITION);
        floorShader->setVec3("light.ambient", lightAmbient);
        floorShader->setVec3("light.diffuse", lightDiffuse);
        floorShader->setVec3("light.specular", lightSpecular);
        floorShader->setFloat("light.constant", lightConstant);
        floorShader->setFloat("light.linear", lightLinear);
        floorShader->setFloat("light.quadratic", lightQuadratic);
        floorShader->setVec3("lightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader->use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Shader Selection Window
        ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - IMGUI_WINDOW_WIDTH - 20, 20), ImGuiCond_Always);
        ImGui::Begin("ShaderDemos", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Model selection dropdown
        ImGui::Text("Model");
        ImGui::Spacing();
        if (ImGui::Combo("##Model", &currentModelIndex, modelNames, IM_ARRAYSIZE(modelNames)))
        {
            // Model selection changed, reload the model
            delete ourModel;
            ourModel = new Model(modelPaths[currentModelIndex]);
            std::cout << "Switched to model: " << modelNames[currentModelIndex] << std::endl;
        }
        ImGui::Spacing();
        ImGui::Spacing();

        // Model shader dropdown
        int previousModelShaderIndex = currentModelShaderIndex;

        ImGui::Text("Model Shader");
        ImGui::Spacing();
        if (ImGui::Combo("##ModelShader", &currentModelShaderIndex, modelShaderNames, IM_ARRAYSIZE(modelShaderNames)))
        {
            // Shader selection changed, reload the shader
            delete modelShader;
            modelShader = new Shader("Shaders/model/model.v", modelShaderPaths[currentModelShaderIndex]);
            std::cout << "Switched to model shader: " << modelShaderNames[currentModelShaderIndex] << std::endl;
        }
        ImGui::Spacing();
        ImGui::Spacing();


        // Post-processing shader dropdown
        ImGui::Text("Post-Processing");
        ImGui::Spacing();
        int previousShaderIndex = currentShaderIndex;
        if (ImGui::Combo("##Post-Processing", &currentShaderIndex, shaderNames, IM_ARRAYSIZE(shaderNames)))
        {
            // Shader selection changed, reload the shader
            delete screenShader;
            screenShader = new Shader("shaders/postProcessing/screen.v", shaderPaths[currentShaderIndex]);
            screenShader->use();
            screenShader->setInt("screenTexture", 0);
            std::cout << "Switched to shader: " << shaderNames[currentShaderIndex] << std::endl;
        }
        ImGui::Spacing();
        ImGui::Spacing();

        // Camera auto-spin toggle
        ImGui::Text("Camera Auto-Spin");
        ImGui::Spacing();
        ImGui::Checkbox("##AutoSpin", &autoSpin);
        ImGui::Spacing();
        ImGui::Spacing();

        // Rotation speed slider (only relevant when auto-spin is on)
        ImGui::Text("Camera Rotation Speed");
        ImGui::Spacing();
        ImGui::SliderFloat("##RotationSpeed", &rotationRate, -0.5f, 0.5f, "%.1f rad/s");
        ImGui::Spacing();
        ImGui::Spacing();

        // Camera position slider (only active when auto-spin is off)
        ImGui::Text("Camera Position");
        ImGui::Spacing();
        if (!autoSpin)
        {
            ImGui::SliderFloat("##CameraPosition", &cameraPosition, 0.0f, 1.0f, "%.3f");
        }
        else
        {
            // Display read-only slider when auto-spin is on
            ImGui::BeginDisabled();
            ImGui::SliderFloat("##CameraPosition", &cameraPosition, 0.0f, 1.0f, "%.3f");
            ImGui::EndDisabled();
        }
        ImGui::Spacing();
        ImGui::Spacing();


        // Rotation radius slider
        ImGui::Text("Zoom");
        ImGui::Spacing();
        ImGui::SliderFloat("##RotationRadius", &radius, 2.0f, 8.0f, "%.1f");
        ImGui::Spacing();
        ImGui::Spacing();

        // Model local color picker
        ImGui::Text("Model Color");
        ImGui::Spacing();
        ImGui::ColorPicker3("##ModelColor", modelLocalColor, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Spacing();

        // Light color picker
        ImGui::Text("Light Color");
        ImGui::Spacing();
        ImGui::ColorPicker3("##LightColor", lightColor, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

        ImGui::End();

        // imgui draw
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &framebuffer);

    delete screenShader;
    delete modelShader;
    delete ourModel;
    delete lightModel;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}