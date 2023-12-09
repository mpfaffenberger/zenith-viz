import numpy as np
import pandas as pd
import shapefile
import cppyy

print(__path__)

countries_shp = shapefile.Reader(
    "/Users/michael.pfaffenberger/Downloads/Countries_WGS84/Countries_WGS84.shp"
)
shapes = countries_shp.shapes()
names = [rec[1] for rec in countries_shp.records()]

polygons = []


def retrieve_parts(shape, idx):
    points = shape.points
    parts = shape.parts
    parts.append(len(points) - 1)
    for i in range(0, len(parts) - 1):
        polygons.append(
            (np.array(points[parts[i] : parts[i + 1] - 1]), names[idx], idx)
        )


def flatten(array: np.array) -> np.array:
    size = array.shape[0]
    flattened = np.zeros(size * 3)
    for idx, step in enumerate(range(0, size * 3, 3)):
        flattened[step] = array[idx, 0]
        flattened[step + 1] = array[idx, 1]
        flattened[step + 2] = 1.0
    return flattened


for idx, shape in enumerate(shapes):
    retrieve_parts(shape, idx)

vertices = [
    (flatten(array), name, idx, array.shape[0]) for array, name, idx in polygons
]

models = [
    GLModel(array.astype("f"), length, 3, 0, 2, name)
    for array, name, idx, length in vertices
]
e = Engine()

for model in models:
    e.models.push_back(model)
read_data_def = """
struct record {
    float lon;
    float lat;
    long time;
};

GLModel* readData() {
    FILE* data;
    data = fopen("/Users/michael.pfaffenberger/code/match.box/signal_data.dat", "r");
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
        timeData[idx] = records[idx].time;
        idx++;
    }
    auto model = new GLModelAnimated(points, numRecords, 3, 0, GL_POINTS, 900, 3600*4, timeData);
    free(records);
    return model;
}
"""
cppyy.cppdef(read_data_def)
from cppyy.gbl import readData

animated_model = readData()
e.models.push_back(animated_model)

l8_data = pd.read_msgpack("landsat-8.msgpack")
l8_data = l8_data.sort_values("time", ascending=True)


flattened = np.vstack(
    (l8_data.x.values, l8_data.y.values, np.ones(len(l8_data.x)))
).ravel(order="F")
l8_model = GLModelAnimated(
    flattened.astype(np.float32),
    len(l8_data.x),
    3,
    0,
    0,
    3600,
    3600,
    l8_data.time.values.astype(np.int64),
    "Landsat 8",
)
e.models.push_back(l8_model)
e.animate()
