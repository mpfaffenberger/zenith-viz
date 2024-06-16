#ifndef ZENITH_CPP_CONTROLS_HPP_
#define ZENITH_CPP_CONTROLS_HPP_

#include <cstdio>
#include <tuple>
#include <vector>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"
#include "vptree.hpp"

class Controls {
 public:
    double last_x;
    double last_y;

    double mouseSpeed;
    GLFWwindow* window;
    static double scrollOffset;
    GLint vertexbuffer;
    Controls(GLFWwindow* window, double mouseSpeed);
    glm::vec3 getTranslationVector(float width, float height);
    virtual glm::mat4 getRotationMatrix(int width, int height);
    std::tuple<int, glm::vec3, float> select(
        glm::mat4 model,
        glm::mat4 view,
        glm::mat4 projection,
        glm::mat4 rotation,
        VpTree<DataPoint, euclidean_distance>* tree_index
    );
    static void scrollCallback(GLFWwindow* window, double x, double y) {
        auto scrollOffsetPointer =
            reinterpret_cast<double*>(glfwGetWindowUserPointer(window));
        if (*scrollOffsetPointer + y > 1.0) {
            *scrollOffsetPointer += y;
        } else {
            double y_sign = y / abs(y);
            double change = y_sign * 0.01;
            if (*scrollOffsetPointer + change > 0.01) {
                *scrollOffsetPointer += change;
            } else {
                *scrollOffsetPointer = 0.01;
            }
        }
    }
};

class Controls3d: public Controls {
 public:
    double last_x;
    double last_y;

    double last_angle_x;
    double last_angle_y;
    double angle_x;
    double angle_y;
    double mouseSpeed;
    double rotateSpeed;
    GLFWwindow* window;
    static double scrollOffset;

    Controls3d(GLFWwindow* window, double mouseSpeed);
    glm::vec3 getTranslationVector(float width, float height);
    glm::mat4 getRotationMatrix(int width, int height);

    static void scrollCallback3d(GLFWwindow* window, double x, double y) {
        auto scrollOffsetPointer =
            reinterpret_cast<double*>(glfwGetWindowUserPointer(window));
        *scrollOffsetPointer -= (y / 2.0);
    }
};

#endif  // ZENITH_CPP_CONTROLS_HPP_
