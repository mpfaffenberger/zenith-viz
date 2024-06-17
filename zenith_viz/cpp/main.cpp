#include "GLModel.hpp"
#include "Engine.hpp"

struct record {
    float lon;
    float lat;
    long time;
};

GLModel* readData() {
    FILE* data;
    data = fopen("/home/mpfaffenberger/code/zenith/zenith/cpp/signal_data.dat", "r");
    fseek(data, 0, SEEK_END);
    auto fileSize = ftell(data);
    auto numRecords = fileSize / sizeof(record);
    auto records = (record*) malloc(sizeof(record) * numRecords);
    fseek(data, 0, 0);
    fread(records, 1, (size_t) fileSize, data);
    fclose(data);
    auto points = (float*) malloc(sizeof(float) * numRecords * 3);
    auto timeData = (long*) malloc(sizeof(long) * numRecords);
    int idx = 0;
    for (int i=0; i < numRecords * 3; i=i+3) {
        points[i] = records[idx].lon;
        points[i+1] = records[idx].lat;
        points[i+2] = 1.0f;
        timeData[idx] = idx;
        printf("%f, %f, %l\n", records[idx].lon, records[idx].lat, records[idx].time);
        idx++;
    }
    auto colorData = (float*) malloc(4 * sizeof(float));
    for (int i=0; i < 4; i++) {
      colorData[i] = 0.7;
    }
    auto stringreps = new std::vector<std::string>();
    auto name = new std::string("stuff");
    printf("make animated model \n");

    auto model = new GLModelAnimated(
        points,
        numRecords,
        3,
        0,
        GL_POINTS,
        1,
        20,
        timeData, *name, colorData, nullptr, false, 0, *stringreps, true);
    free(records);
    return model;
}


int main() {
  auto shaderPath = new std::string("/home/mpfaffenberger/code/zenith/zenith/shaders");
  auto engine = new Engine(*shaderPath);
  auto model = readData();
  engine->models->insert(std::pair<int, GLModel*>(0, model));
  engine->animate();
};
