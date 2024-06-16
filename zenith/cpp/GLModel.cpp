#ifndef ZENITH_GLMODEL_C
#define ZENITH_GLMODEL_C

#include "GLModel.hpp"
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cstdlib>
#include "vector"
#include "imgui/imgui.h"

GLModel::GLModel(const float* vertexData, int numVertices, int numComponents, int stride,
                 GLuint drawType, std::string name, const float* color, const float* colordata, int useColorData, int id,
                 std::vector<std::string> stringReps, bool pickingEnabled) {
    this->pickingEnabled = pickingEnabled;
    this->drawStyles = new std::vector<std::string*>;
    this->drawStyles->push_back(new std::string("GL_POINTS"));
    this->drawStyles->push_back(new std::string("GL_LINES"));
    this->drawStyles->push_back(new std::string("GL_LINE_LOOP"));
    this->drawStyles->push_back(new std::string("GL_LINE_STRIP"));
    this->drawStyles->push_back(new std::string("GL_TRIANGLES"));
    this->drawStyles->push_back(new std::string("GL_TRIANGLE_STRIP"));
    this->drawStyles->push_back(new std::string("GL_TRIANGLE_FAN"));
    this->drawStyles->push_back(new std::string("GL_QUADS"));
    this->drawStyles->push_back(new std::string("GL_QUAD_STRIP"));
    this->drawStyles->push_back(new std::string("GL_POLYGON"));
    this->size = 3.0f;
    this->vertexData = (float*) malloc(sizeof(float) * numVertices * numComponents);
    for (int i=0; i < numVertices * numComponents; i++) {
        this->vertexData[i] = vertexData[i];
    }

    this->useColorData = useColorData;

    if (useColorData > 0) {
        this->colorData = (float*) malloc(sizeof(float) * numVertices * numComponents);
        for (int i=0; i < numVertices * numComponents; i++) {
            this->colorData[i] = colordata[i];
        }
    }

    if (pickingEnabled) {
        this->tree_index = new VpTree<DataPoint, euclidean_distance>();
        this->idxVertices = new std::vector<DataPoint>();
        int vertexIdx = 0;
        for (int i = 0; i < numVertices * numComponents; i = i + 3) {
            auto point = (float *) malloc(sizeof(float) * 3);
            point[0] = vertexData[i];
            point[1] = vertexData[i + 1];
            point[2] = vertexData[i + 2];
            auto datapoint = new DataPoint(3, vertexIdx, point);
            this->idxVertices->push_back(*datapoint);
            free(point);
            vertexIdx++;
        }
        this->tree_index->create(*this->idxVertices);
        this->pickingEnabled = true;
    }

    this->numVertices = numVertices;
    this->numComponents = numComponents;
    this->stride = stride;
    this->drawType = drawType;
    this->color = (float*) malloc(sizeof(float)*4);
    for (int i = 0; i < 4; i++) {
        this->color[i] = color[i];
    }
    this->name = name;
    this->id = id;
    this->stringReps = stringReps;
    this->bufferInitialized = false;
}

GLModel::~GLModel() {
    free(this->vertexData);
    free(this->color);
    this->drawStyles->clear();
    if (bufferInitialized)
        glDeleteBuffers(1, &this->vertexBuffer);
    this->bufferInitialized = false;
    if (this->pickingEnabled) {
        this->idxVertices->clear();
        delete this->tree_index;
    }

    if (this->useColorData) {
        glDeleteBuffers(1, &this->colorBuffer);
        free(this->colorData);
    }
}

void GLModel::initBuffer() {
    if (this->bufferInitialized){
        glDeleteBuffers(1, &this->vertexBuffer);
    }
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * numComponents, vertexData, GL_STATIC_DRAW);
    this->vertexBuffer = vertexbuffer;

    if (this->useColorData) {
        GLuint vertexColorBuffer;
        glGenBuffers(1, &vertexColorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * numComponents, this->colorData, GL_STATIC_DRAW);
        this->colorBuffer = vertexColorBuffer;
    }
    this->bufferInitialized = true;
}

void GLModel::bindVertexBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
}

void GLModel::render(GLuint shaderProgram) {
    ImGui::BeginChild(this->name.c_str(), ImVec2(400, 65));
    ImGui::Text(
        "Model Name: %s, Vertices: %d, draw-type: %s",
        this->name.c_str(),
        this->numVertices,
        this->drawStyles->at(this->drawType)->c_str()
    );
    ImGui::ColorEdit4(this->name.c_str(), this->color);
    ImGui::SliderFloat("size", &size, 0.0f, 40.0f);
    ImGui::EndChild();
    ImGui::End();
    glEnableVertexAttribArray(0);
    GLint colorVar = glGetUniformLocation(shaderProgram, "color");
    GLint sizeVar = glGetUniformLocation(shaderProgram, "point_size");
    GLint useColor = glGetUniformLocation(shaderProgram, "use_color_data");

    glUniform4f(colorVar, (GLfloat) color[0], (GLfloat) color[1], (GLfloat) color[2], (GLfloat) color[3]);
    glUniform1f(sizeVar, (GLfloat) size);

    if (this->useColorData)
        glUniform1f(useColor, (GLfloat) 1.0f);
    else
        glUniform1f(useColor, (GLfloat) 0.0f);

    this->bindVertexBuffer();
    glVertexAttribPointer(
        0,
        this->numComponents,
        GL_FLOAT,
        GL_FALSE,
        this->stride,
        nullptr
    );

    glDrawArrays(this->drawType, 0, this->numVertices);
    if (this->useColorData) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, this->colorBuffer);
        glVertexAttribPointer(
            1,
            this->numComponents,
            GL_FLOAT,
            GL_FALSE,
            this->stride,
            nullptr
        );
    }
    glDrawArrays(this->drawType, 0, this->numVertices);
    if (this->useColorData)
        glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

GLModelAnimated::GLModelAnimated(
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
): GLModel::GLModel(vertexData, numVertices, numComponents, stride, drawType, name, color, colordata, useColorData, id, stringReps, pickingEnabled) {
    this->timeData = (long*) malloc(sizeof(long) * numVertices);
    for (int i=0; i < numVertices; i++) {
        this->timeData[i] = timeData[i];
    }
    this->windowSize = windowSize;
    this->time = glfwGetTime();
    this->curIndex = 0;
    this->stepSize = stepSize;
    this->createTimeSteps();
    this->fps = 30.0f;
    this->endStep = 1;
    this->windowSteps = 1;
    this->paused = true;
}

GLModelAnimated::~GLModelAnimated() {
    free(this->timeData);
    free(this->startOffsets);
    free(this->endOffsets);
}

void GLModelAnimated::timeUpdate(int next) {
    if (this->curIndex >= (this->numSteps - 1)) {
        this->curIndex = 0;
    } else {
        this->curIndex = this->curIndex + next;
    }
    int last = this->curIndex + this->windowSteps;
    if (last > this->numSteps - 1) {
        last = this->numSteps - 1;
    }
    this->endStep = last;
}

void GLModelAnimated::render(GLuint shaderProgram) {
    ImGui::Begin("Animated Models");
    ImGui::BeginChild(this->name.c_str(), ImVec2(450, 175));
    ImGui::Text(
        "Model Name: %s, Vertices: %d, draw-type: %s",
        this->name.c_str(),
        this->numVertices,
        drawStyles->at(this->drawType)->c_str()
    );

    ImGui::ColorEdit4(this->name.c_str(), this->color);
    ImGui::SliderFloat("size", &size, 0.0f, 40.0f);
    ImGui::SliderInt("Begin Step", (int *) &this->curIndex, 0, this->numSteps-1);
    ImGui::SliderInt("Window Steps", (int *) &this->windowSteps, 1, this->numSteps-2);
    ImGui::SliderFloat("FPS", &this->fps, 1.0, 200.0f);

    if (ImGui::Button("Play/Pause")) {
        paused = !paused;
    }
    ImGui::EndChild();
    ImGui::End();
    double curTime = glfwGetTime();

    if (curTime > this->time + (1.0f / this->fps) && !paused) {
        timeUpdate(1);
        this->time = curTime;
    } else {
        if (paused) {
           timeUpdate(0);
        }
    }

    glEnableVertexAttribArray(0);
    GLint colorVar = glGetUniformLocation(shaderProgram, "color");
    GLint sizeVar = glGetUniformLocation(shaderProgram, "point_size");
    GLint useColor = glGetUniformLocation(shaderProgram, "use_color_data");

    glUniform4f(colorVar, (GLfloat) color[0], (GLfloat) color[1], (GLfloat) color[2], (GLfloat) color[3]);
    glUniform1f(sizeVar, (GLfloat) size);

    if (useColorData)
        glUniform1f(useColor, (GLfloat) 1.0f);
    else
        glUniform1f(useColor, (GLfloat) 0.0f);

    glEnableVertexAttribArray(0);
    this->bindVertexBuffer();
    auto start = (GLuint) this->startOffsets[this->curIndex];
    auto stop = (GLuint) this->endOffsets[this->endStep - 1];
    glVertexAttribPointer(
        0,
        this->numComponents,
        GL_FLOAT,
        GL_FALSE,
        this->stride,
        nullptr
    );

    if (this->useColorData) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, this->colorBuffer);
        glVertexAttribPointer(
            1,
            this->numComponents,
            GL_FLOAT,
            GL_FALSE,
            this->stride,
            nullptr
        );
    }
    glDrawArrays(this->drawType, start, stop - start);
    if (this->vertexBuffer)
        glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

}

void GLModelAnimated::createTimeSteps() {
    long minTime = timeData[0];
    long maxTime = timeData[numVertices - 1];
    this->numSteps = (maxTime - minTime) / stepSize;
    this->startOffsets = (unsigned int*) malloc(sizeof(unsigned int) * numSteps);
    this->endOffsets = (unsigned int*) malloc(sizeof(unsigned int) * numSteps);
    long curTime = minTime;
    this->startOffsets[0] = 0;
    int stepIdx = 1;
    for (unsigned int i=0; i < numVertices; i++) {
        long recordTime = timeData[i];
        if (curTime + stepSize < recordTime) {
            this->startOffsets[stepIdx] = i;
            curTime = curTime + stepSize;
            stepIdx++;
        }
    }
    stepIdx = 0;
    curTime = minTime;
    for (unsigned int i=0; i < numVertices; i++) {
        long recordTime = timeData[i];
        if (curTime + stepSize + windowSize < recordTime) {
            this->endOffsets[stepIdx] = i;
            curTime = curTime + stepSize;
            stepIdx++;
        }
    }
}
#endif
