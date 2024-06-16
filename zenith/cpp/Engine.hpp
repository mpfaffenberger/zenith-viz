#ifndef ZENITH_CPP_ENGINE_HPP_
#define ZENITH_CPP_ENGINE_HPP_

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

class Engine {
 public:
    std::string shaderPath;
    std::map<int, GLModel*> *models;
    Controls *controls;
    GLFWwindow *window;
    GLBoilerPlate *bp;

    glm::mat4 modelAffine;

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram;
    GLuint vertexArrayId;
    GLint projectionMatrix;
    float mouseSpeed;

    float *bgcolor;
    int width;
    int height;

    bool vertexArrayInitialized = false;
    bool shaderInitialized = false;
    glm::vec3 picking_point;


    explicit Engine(std::string shaderPath);
    ~Engine();
    virtual void initControls();
    void initialize();
    void deinitialize();
    void renderSubroutine(
        glm::mat4 modelViewProjection,
        glm::mat4 model, glm::mat4 view,
        glm::mat4 projection,
        glm::mat4 rotation
    );
    virtual void animate();
    bool addModel(int id, GLModel* model);
    bool removeModel(int id);
    bool modelExists(int id);
    int numModels();
};

class Engine3d: public Engine {
 public:
    glm::vec3 camPosition;
    glm::vec3 camLookAt;

    explicit Engine3d(std::string shaderPath);
    void initControls() override;
    void animate() override;
};

static Engine* create2dWorld(std::string shaderPath) {
    return new Engine(shaderPath);
}

static Engine3d* create3dWorld(std::string shaderPath) {
    return new Engine3d(shaderPath);
}


#endif  // ZENITH_CPP_ENGINE_HPP_
