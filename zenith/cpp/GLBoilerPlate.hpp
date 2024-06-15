#ifndef ZENITH_GLBOILERPLATE_H
#define ZENITH_GLBOILERPLATE_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Controls.hpp"
#include "GLModel.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

class GLBoilerPlate {
public:
    GLFWwindow* initWindow();
    void render(GLFWwindow *window, std::map<int, GLModel*>* models, GLuint shaderProgram, GLint matrixId, glm::mat4 mvp);
    GLuint compileShader(const char* file_path, GLenum shaderType);
    GLuint linkShaders(GLuint vertexProgram, GLuint fragmentProgram);
    void checkProgram(GLuint program, GLenum pname);
};
#endif //ZENITH_GLBOILERPLATE_H
