#ifndef ZENITH_GLBOILERPLATE_H
#define ZENITH_GLBOILERPLATE_H

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <map>
#include "GLModel.hpp"

class GLBoilerPlate {
public:
    GLFWwindow* initWindow();
    void render(GLFWwindow *window, std::map<int, GLModel*>* models, GLuint shaderProgram, GLint matrixId, glm::mat4 mvp);
    GLuint compileShader(const char* file_path, GLenum shaderType);
    GLuint linkShaders(GLuint vertexProgram, GLuint fragmentProgram);
    void checkProgram(GLuint program, GLenum pname);
};
#endif //ZENITH_GLBOILERPLATE_H
