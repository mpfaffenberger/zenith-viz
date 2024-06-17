#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "Engine.hpp"
#include "GLModel.hpp"


namespace py = pybind11;

GLModel* create_gl_model(
    py::array_t<float> vertex_data,
    int num_vertices,
    int num_components,
    int stride,
    int draw_type,
    std::string name,
    py::array_t<float> color,
    py::array_t<float> color_data,
    int use_color_data,
    int id,
    std::vector<std::string> string_reps,
    bool picking_enabled
) {
    const float* vertex_data_ptr = static_cast<const float*>(vertex_data.data());
    const float* color_ptr = static_cast<const float*>(color.data());
    const float* color_data_ptr = static_cast<const float*>(color_data.data());
    auto model = new GLModel(
        vertex_data_ptr,
        num_vertices,
        num_components,
        stride,
        draw_type,
        name,
        color_ptr,
        color_data_ptr,
        use_color_data,
        id,
        string_reps,
        picking_enabled
    );
    return model;
}


GLModelAnimated* create_gl_model_animated(
    py::array_t<float> vertex_data,
    int num_vertices,
    int num_components,
    int stride,
    int draw_type,
    unsigned int step_size,
    unsigned int window_size,
    py::array_t<long> time_data,
    std::string name,
    py::array_t<float> color,
    py::array_t<float> color_data,
    int use_color_data,
    int id,
    std::vector<std::string> string_reps,
    bool picking_enabled
) {
    const float* vertex_data_ptr = static_cast<const float*>(vertex_data.data());
    const float* color_ptr = static_cast<const float*>(color.data());
    const float* color_data_ptr = static_cast<const float*>(color_data.data());
    const long* time_data_ptr = static_cast<const long*>(time_data.data());
    auto model = new GLModelAnimated(
        vertex_data_ptr,
        num_vertices,
        num_components,
        stride,
        draw_type,
        step_size,
        window_size,
        time_data_ptr,
        name,
        color_ptr,
        color_data_ptr,
        use_color_data,
        id,
        string_reps,
        picking_enabled
    );
    return model;
}

PYBIND11_MODULE(_zenith, m) {
    py::class_<Engine>(m, "Engine")
        .def(py::init<const std::string &>())
        .def("animate", &Engine::animate)
        .def("add_model", &Engine::addModel)
        .def("remove_model", &Engine::removeModel)
        .def("model_exists", &Engine::modelExists)
        .def("num_models", &Engine::numModels);
    py::class_<Engine3d>(m, "Engine3d")
        .def(py::init<const std::string &>())
        .def("animate", &Engine::animate)
        .def("add_model", &Engine::addModel)
        .def("remove_model", &Engine::removeModel)
        .def("model_exists", &Engine::modelExists)
        .def("num_models", &Engine::numModels);

    py::class_<GLModel>(m, "GLModel")
        .def("name", [](GLModel* model){ return model->name; });

    py::class_<GLModelAnimated>(m, "GLModelAnimated")
        .def("name", [](GLModel* model){ return model->name; });

    m.def(
        "create_gl_model",
        &create_gl_model,
        "Create a 2d or 3d model",
        py::arg("vertex_data"),
        py::arg("num_vertices"),
        py::arg("num_components"),
        py::arg("stride"),
        py::arg("draw_type"),
        py::arg("name"),
        py::arg("color"),
        py::arg("color_data"),
        py::arg("use_color_data"),
        py::arg("id"),
        py::arg("string_reps"),
        py::arg("picking_enabled")
    );

    m.def(
        "create_gl_model_animated",
        &create_gl_model_animated,
        "Create a 2d or 3d model",
        py::arg("vertex_data"),
        py::arg("num_vertices"),
        py::arg("num_components"),
        py::arg("stride"),
        py::arg("draw_type"),
        py::arg("step_size"),
        py::arg("window_size"),
        py::arg("time_data"),
        py::arg("name"),
        py::arg("color"),
        py::arg("color_data"),
        py::arg("use_color_data"),
        py::arg("id"),
        py::arg("string_reps"),
        py::arg("picking_enabled")
    );
}
