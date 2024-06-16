#ifndef ZENITH_CPP_ENGINE_CPP_
#define ZENITH_CPP_ENGINE_CPP_

using namespace std;
#include "Engine.hpp"
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <tuple>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "GLBoilerPlate.hpp"
#include "Controls.hpp"
#include "GLModel.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"


Engine::Engine(std::string shaderPath) {
    models = new std::map<int, GLModel*>();
    mouseSpeed = 20.0f;
    this->shaderPath = shaderPath;
    picking_point = glm::vec3(0.0f, 0.0f, 0.0f);
}

Engine::~Engine() {
    if (vertexArrayInitialized) {
        glDeleteVertexArrays(1, &vertexArrayId);
        vertexArrayInitialized = false;
    }
    if (shaderInitialized)
        glDeleteShader(shaderProgram);
}

void Engine::initControls() {
    this->controls = new Controls(this->window, this->mouseSpeed);
}

void Engine::initialize() {
    this->width = 1024;
    this->height = 768;
    this->bp = new GLBoilerPlate();
    this->window = bp->initWindow();
    initControls();
    if (!shaderInitialized) {
        vertexShader = bp->compileShader(
            (this->shaderPath + "/vertexShader.shader").c_str(),
            GL_VERTEX_SHADER);

        fragmentShader = bp->compileShader(
            (this->shaderPath + "/fragmentShader.shader").c_str(),
            GL_FRAGMENT_SHADER);

        shaderProgram = bp->linkShaders(vertexShader, fragmentShader);
        shaderInitialized = true;
    }
    projectionMatrix = glGetUniformLocation(shaderProgram, "MVP");
    modelAffine = glm::mat4(1.0f);
    glGenVertexArrays(1, &vertexArrayId);
    glBindVertexArray(vertexArrayId);
    vertexArrayInitialized = true;
    for (auto && pair : *models) {
        pair.second->initBuffer();
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    bgcolor = reinterpret_cast<float*>(malloc(sizeof(float) * 4));
    for (int i=0; i < 4; i++) bgcolor[i] = 0.05f;
    glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}


void Engine::deinitialize() {
    delete controls;
    delete bp;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteShader(shaderProgram);
    this->shaderInitialized = false;

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Engine::renderSubroutine(
    glm::mat4 modelViewProjection,
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection,
    glm::mat4 rotation) {

    glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    GLint pick_var = glGetUniformLocation(shaderProgram, "picking_point");
    glUniform3f(pick_var, picking_point.x, picking_point.y, picking_point.z);

    bp->render(
        window,
        models,
        shaderProgram,
        projectionMatrix,
        modelViewProjection);

    ImGui::Begin("Info box");
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Data Items");
    ImGui::BeginChild("Scrolling");
    GLModel* best_model;
    auto best_id = -1;
    float best_dist = 1000000.0f;
    for (auto && gl_model_pair : *models) {
        auto gl_model = gl_model_pair.second;
        if (gl_model->pickingEnabled) {
            auto selection = controls->select(
                model, view, projection,
                rotation, gl_model->tree_index);

            auto id = std::get<0>(selection);
            if (id >= 0) {
                auto point = std::get<1>(selection);
                auto distance = std::get<2>(selection);
                if (distance < best_dist) {
                    picking_point.x = point.x;
                    picking_point.y = point.y;
                    picking_point.z = point.z;
                    best_dist = distance;
                    best_model = gl_model;
                    best_id = id;
                }
            }
        }
    }
    if (best_id > -1) {
        ImGui::Text("Model Name: %s", best_model->name.c_str());
        ImGui::Text("Index: %d", best_id);
        if (best_model->stringReps.size() > 0) {
            ImGui::Text("Data: %s", best_model->stringReps[best_id].c_str());
        }
    }
    ImGui::EndChild();
    ImGui::End();
    ImGui::Begin("Control Panel");
    ImGui::ColorEdit4("Background Color", bgcolor);
    ImGui::End();
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Engine::addModel(int id, GLModel *model) {
    this->models->insert(std::pair<int, GLModel*>(id, model));
    return true;
}

bool Engine::removeModel(int id) {
    if (this->modelExists(id)) {
        this->models->erase(id);
        return true;
    }
    return false;
}

bool Engine::modelExists(int id) {
    return this->models->find(id) != this->models->end();
}

int Engine::numModels() {
    return this->models->size();
}

void Engine::animate() {
    initialize();
    float slider = 0.0f;
    do {
        double scrollFactor;
        double magnitude = (width + height) / 2.0;
        scrollFactor = Controls::scrollOffset / magnitude;

        glfwGetFramebufferSize(window, &width, &height);
        mouseSpeed = magnitude / (1 / scrollFactor);
        controls->mouseSpeed = mouseSpeed;
        glm::vec3 position = glm::vec3(0, 0, 5);
        float horizontalAngle = 3.14f;
        float verticalAngle = 0.0f;
        float initialFoV = 90.0f;
        glm::vec3 direction(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle));
        glm::vec3 right = glm::vec3(
            sin(horizontalAngle - 3.14f / 2.0f),
            0,
            cos(horizontalAngle - 3.14f / 2.0f));
        glm::vec3 up = glm::cross(right, direction);
        double world_left = -(width / 2.0) * scrollFactor;
        double world_right = (width / 2.0) * scrollFactor;
        double world_top = (height / 2.0) * scrollFactor;
        double world_bottom = -(height / 2.0) * scrollFactor;
        glm::mat4 projection = glm::ortho<double>(
            world_left,
            world_right,
            world_bottom,
            world_top,
            0.9f,
            100.0f);

        auto view = glm::lookAt(
            position,
            position + direction,
            up);

        glm::vec3 translation = controls->getTranslationVector(width, height);
        modelAffine = glm::translate(modelAffine, translation);
        glm::mat4 mvp = projection * view * modelAffine;
        renderSubroutine(mvp, modelAffine, view, projection, glm::mat4(1.0f));
    } while ((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0));

    deinitialize();
}



Engine3d::Engine3d(std::string shaderPath): Engine(shaderPath) {
    models = new std::map<int, GLModel*>();
    mouseSpeed = 20.0f;
    camPosition = glm::vec3(0.0f, 0.0f, 5.0f);
    camLookAt = glm::vec3(0.0f, 0.0f, -1000.0f);
}

void Engine3d::initControls() {
    this->controls = new Controls3d(this->window, this->mouseSpeed);
}

void Engine3d::animate() {
    initialize();
    float fov = 90.0f;
    do {
        float scrollFactor;
        scrollFactor = Controls::scrollOffset;
        glfwGetFramebufferSize(window, &width, &height);

        mouseSpeed = abs(scrollFactor) * static_cast<float>(width)
            / static_cast<float>(height);

        controls->mouseSpeed = mouseSpeed;
        glm::mat4 projection = glm::perspective(
            fov,
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            1000.0f);

        glm::vec3 translation = controls->getTranslationVector(width, height);
        glm::mat4 rotation = controls->getRotationMatrix(width, height);
        modelAffine = glm::translate(modelAffine, translation);
        camPosition.z = scrollFactor;
        glm::mat4 view = glm::lookAt(
            camPosition,
            camLookAt,
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 mvp = projection * view * modelAffine * rotation;
        renderSubroutine(mvp, modelAffine, view, projection, rotation);
    } while ((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0));

    deinitialize();
}
#endif
