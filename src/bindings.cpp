// src/bindings.cpp

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Forward declarations of binding functions
void bind_common(py::module& m);
void bind_controller(py::module& m);
void bind_udp(py::module& m);

PYBIND11_MODULE(_abb_libegm, m) {
    m.doc() = "Python bindings for ABB LibEGM - External Guided Motion interface";

    // Initialize the submodules
    bind_common(m);
    bind_controller(m);
    bind_udp(m);
}