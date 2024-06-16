#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include "GLBoilerPlate.hpp"
#include <iostream>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Controls.hpp"
#include "GLModel.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"


void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void glfwError(int id, const char* description) {
    std::cout << description << std::endl;
}

GLFWwindow* GLBoilerPlate::initWindow() {


    glfwSetErrorCallback(&glfwError);
    if (!glfwInit()) {
        fprintf(stderr, "Couldn't initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Zenith", nullptr, nullptr);
    if( window == nullptr ) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible." );
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    gladLoadGL(glfwGetProcAddress);
    return window;
};

void GLBoilerPlate::render(GLFWwindow *window, std::map<int, GLModel*>* models, GLuint shaderProgram, GLint matrixId, glm::mat4 mvp) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);
    GLint resolutionVar = glGetUniformLocation(shaderProgram, "u_resolution");
    GLint mouseVar = glGetUniformLocation(shaderProgram, "u_mouse");
    GLint timeVar = glGetUniformLocation(shaderProgram, "u_time");

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    int width = 0;
    int height = 0;

    glfwGetWindowSize(window, &width, &height);
    float resData[] = {
            (float) width,
            (float) height
    };

    float fb_factor_x = (float) fb_width / width;
    float fb_factor_y = (float) fb_height / height;

    GLint isPoint = glGetUniformLocation(shaderProgram, "is_point");

    double mouse_x = 0.0;
    double mouse_y = 0.0;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    float mouseData[] = {
            (float) (mouse_x / width) * fb_factor_x,
            (float) ((height - (float) mouse_y) / height) * fb_factor_y
    };

    glUniform2fv(resolutionVar, 1, resData);
    glUniform2fv(mouseVar, 1, mouseData);

    for (auto && kvPair : *models) {
        if (kvPair.second->drawType == GL_POINTS) {
            glUniform1i(isPoint, 1);
        } else {
            glUniform1i(isPoint, 0);
        }
        kvPair.second->render(shaderProgram);
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
};

GLuint GLBoilerPlate::compileShader(const char* file_path, GLenum shaderType) {
    GLuint shaderId = glCreateShader(shaderType);
    std::string shaderCode;
    std::ifstream shaderStream(file_path, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    } else {
        getchar();
        return 0;
    }
    char const* sourcePointer = shaderCode.c_str();
    glShaderSource(shaderId, 1, &sourcePointer, nullptr);
    glCompileShader(shaderId);
    this->checkProgram(shaderId, GL_COMPILE_STATUS);

    return shaderId;
}

GLuint GLBoilerPlate::linkShaders(GLuint vertexProgram, GLuint fragmentProgram) {
    GLuint programId = glCreateProgram();

    glAttachShader(programId, vertexProgram);
    glAttachShader(programId, fragmentProgram);
    glLinkProgram(programId);

    this->checkProgram(programId, GL_LINK_STATUS);

    glDetachShader(programId, vertexProgram);
    glDetachShader(programId, fragmentProgram);

    glDeleteShader(vertexProgram);
    glDeleteShader(fragmentProgram);

    return programId;
}

void GLBoilerPlate::checkProgram(GLuint program, GLenum pname) {

    GLint result = GL_FALSE;
    int infoLogLength;

    if (pname == GL_LINK_STATUS) {
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    } else {
        glGetShaderiv(program, GL_COMPILE_STATUS, &result);
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    }

    if (infoLogLength > 0) {
        std::vector<char> msg(infoLogLength + (uint64_t) 1);
        if (pname == GL_LINK_STATUS)
            glGetProgramInfoLog(program, infoLogLength, nullptr, &msg[0]);
        else
            glGetShaderInfoLog(program, infoLogLength, nullptr, &msg[0]);
        printf("%s\n", &msg[0]);
    }
}