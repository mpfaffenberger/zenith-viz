#include <pybind11/pybind11.h>
#include "Engine.hpp"


namespace py = pybind11;

PYBIND11_MODULE(zenith_, m) {
    py::class_<Engine>(m, "Engine")
        .def(py::init<const std::string &>())
        .def("animate", &Engine::animate);
}
