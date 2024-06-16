#ifndef ZENITH_CPP_CONTROLS_CPP_
#define ZENITH_CPP_CONTROLS_CPP_

#include "Controls.hpp"
#include <tuple>
#include <vector>
#include "glad/gl.h"
#include <glm/gtc/matrix_transform.hpp>

double Controls::scrollOffset = 100.0;

Controls::Controls(GLFWwindow* window, double mouseSpeed) {
    last_x = 0.0;
    last_y = 0.0;
    glfwGetCursorPos(window, &last_x, &last_y);
    this->window = window;
    this->mouseSpeed = mouseSpeed;
    glfwSetWindowUserPointer(window, &Controls::scrollOffset);
    glfwSetScrollCallback(window, scrollCallback);
}

glm::mat4 Controls::getRotationMatrix(int width, int height) {
    return glm::mat4(1.0f);
}

glm::vec3 Controls::getTranslationVector(float width, float height) {
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int shiftState = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

    if (state == GLFW_RELEASE) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        last_x = xpos;
        last_y = ypos;
    }

    if (state == GLFW_PRESS && shiftState != GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        double change_x =
            static_cast<float>(((last_x - xpos) / width) * mouseSpeed);
        double change_y =
            static_cast<float>(((last_y - ypos) / height) * mouseSpeed);

        last_x = xpos;
        last_y = ypos;

        glm::vec3 translation(
            -change_x,
            change_y,
            0);

        return translation;
    } else {
        return glm::vec3(0.0f);
    }
}

Controls3d::Controls3d(
    GLFWwindow* window,
    double mouseSpeed
): Controls::Controls(window, mouseSpeed) {
    last_x = 0.0;
    last_y = 0.0;
    rotateSpeed = 3.0f;
    last_angle_x = 0.0;
    last_angle_y = 0.0;
    angle_x = 0.0;
    angle_y = 0.0;
    glfwGetCursorPos(window, &last_x, &last_y);
    this->window = window;
    this->mouseSpeed = mouseSpeed;
    glfwSetWindowUserPointer(window, &Controls::scrollOffset);
    glfwSetScrollCallback(window, scrollCallback3d);
}

glm::mat4 Controls3d::getRotationMatrix(int width, int height) {
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    glm::mat4 trans = glm::mat4(1.0f);
    double xpos,  ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (state == GLFW_PRESS) {
        double change_x =
            static_cast<float>(((last_x - xpos) / width) * rotateSpeed);
        double change_y =
            static_cast<float>(((last_y - ypos) / height) * rotateSpeed);
        angle_x += change_x;
        angle_y += change_y;
    }
    last_angle_x = angle_x;
    last_angle_y = angle_y;
    last_x = xpos;
    last_y = ypos;
    auto x_rot =
        glm::rotate<float>(trans, angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
    auto y_rot =
        glm::rotate<float>(trans, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
    return x_rot * y_rot;
}

std::tuple<int, glm::vec3, float> Controls::select(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection,
    glm::mat4 rotation,
    VpTree<DataPoint, euclidean_distance>* tree_index
) {
    double x;
    double y;
    int b_w;
    int b_h;
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    glfwGetFramebufferSize(window, &b_w, &b_h);
    glfwGetCursorPos(window, &x, &y);
    float x_ratio = static_cast<float>(width) / static_cast<float>(b_w);
    float y_ratio = static_cast<float>(height) / static_cast<float>(b_h);
    x = x / x_ratio;
    y = y / y_ratio;
    float xpos = x;
    float ypos = (height / y_ratio) - 1 - y;
    float depth = 0.0f;
    glReadPixels(xpos, ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    auto win_v1 = glm::vec3(xpos, ypos, depth);

    int view_x = static_cast<int>(static_cast<float>(width) / x_ratio);
    int view_y = static_cast<int>(static_cast<float>(height) / y_ratio);

    auto unprj_v1 = glm::unProject(
        win_v1,
        view * model * rotation,
        projection,
        glm::vec4(0, 0, view_x, view_y));

    auto results = new std::vector<DataPoint>();
    auto distances = new std::vector<float>();

    float query_point[3] = {
        unprj_v1.x,
        unprj_v1.y,
        unprj_v1.z
    };

    auto dp = new DataPoint(3, 0, query_point);

    tree_index->search(*dp, 1, results, distances);
    delete dp;
    if (distances->at(0) < 0.5f) {
        auto data_item = results->at(0);
        auto data_point = data_item._x;
        auto id = data_item.index();
        auto new_vec = glm::vec3(
            data_point[0],
            data_point[1],
            data_point[2]);

        auto result = std::make_tuple(id, new_vec, distances->at(0));

        results->clear();
        distances->clear();
        return result;
    } else {
        auto result = std::make_tuple(
            -1000,
            glm::vec3(-1000.0f, -1000.0f, -1000.0f),
            1000.0f);

        results->clear();
        distances->clear();
        return result;
    }
}
#endif
