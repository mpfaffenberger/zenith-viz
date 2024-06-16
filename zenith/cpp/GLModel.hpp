#ifndef ZENITH_GLMODEL_H
#define ZENITH_GLMODEL_H

#include <string>
#include <cstdlib>
#include <cstdio>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include "vector"
#include "imgui/imgui.h"
#include "Controls.hpp"

class GLModel {
public:
    int id;
    std::vector<std::string*>* drawStyles;
    std::string name;
    std::vector<std::string> stringReps;
    GLuint colorBuffer;
    GLuint vertexBuffer;
    GLuint drawType;

    int numVertices;
    int numComponents;
    int stride;

    float* color;
    float* colorData;
    float* vertexData;
    float size;

    int useColorData;
    bool bufferInitialized;
    bool pickingEnabled;

    std::vector<DataPoint>* idxVertices;
    VpTree<DataPoint, euclidean_distance>* tree_index;

    GLModel(
        const float* vertexData,
        int numVertices,
        int numComponents,
        int stride,
        GLuint drawType,
        std::string name,
        const float* color,
        const float* colordata,
        int useColorData,
        int id,
        std::vector<std::string> stringReps,
        bool pickingEnabled
    );

    ~GLModel();
    void initBuffer();
    void bindVertexBuffer();
    virtual void render(GLuint shaderProgram);
};

class GLModelAnimated: public GLModel {
public:
    int endStep;
    unsigned int curIndex;
    unsigned int stepSize;
    unsigned int windowSize;
    unsigned int windowSteps;
    unsigned int* startOffsets;
    unsigned int* endOffsets;

    long* timeData;
    long numSteps;

    float fps;
    bool paused;

    double time;

    GLModelAnimated(
        const float* vertexData,
        int numVertices,
        int numComponents,
        int stride,
        GLuint drawType,
        unsigned int stepSize,
        unsigned int windowSize,
        const long* timeData,
        std::string name,
        const float* color,
        const float* colordata,
        int useColorData,
        int id,
        std::vector<std::string> stringReps,
        bool pickingEnabled
    );

    ~GLModelAnimated();
    void timeUpdate(int next);
    void render(GLuint shaderProgram);
    void createTimeSteps();
};
#endif //ZENITH_GLMODEL_H
