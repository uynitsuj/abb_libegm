// src/controller.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "abb_libegm/egm_controller_interface.h"

namespace py = pybind11;

void bind_controller(py::module& m) {
    // Use io_context instead of deprecated io_service
    py::class_<boost::asio::io_context>(m, "IOService")
        .def(py::init<>())
        .def("run", static_cast<std::size_t (boost::asio::io_context::*)()>(&boost::asio::io_context::run))
        .def("stop", &boost::asio::io_context::stop)
        .def("reset", &boost::asio::io_context::reset);

    // Wrap EGMControllerInterface
    py::class_<abb::egm::EGMControllerInterface, abb::egm::EGMBaseInterface>(m, "EGMControllerInterface")
        .def(py::init<boost::asio::io_service&, const unsigned short, const abb::egm::BaseConfiguration&>(),
             py::arg("io_service"),
             py::arg("port_number"),
             py::arg("configuration") = abb::egm::BaseConfiguration())
        .def("waitForMessage", 
             &abb::egm::EGMControllerInterface::waitForMessage,
             py::arg("timeout_ms") = 0,
             "Wait for the next EGM message with optional timeout")
        .def("read", 
             &abb::egm::EGMControllerInterface::read,
             py::arg("inputs"),
             "Read EGM inputs received from the robot controller")
        .def("write",
             &abb::egm::EGMControllerInterface::write,
             py::arg("outputs"),
             "Write EGM outputs to send to the robot controller")
        .def("isInitialized",
             &abb::egm::EGMControllerInterface::isInitialized,
             "Check if the interface is initialized")
        .def("isConnected",
             &abb::egm::EGMControllerInterface::isConnected,
             "Check if connected to the robot controller")
        .def("getStatus",
             &abb::egm::EGMControllerInterface::getStatus,
             "Get current EGM status")
        .def("getConfiguration",
             &abb::egm::EGMControllerInterface::getConfiguration,
             "Get current configuration")
        .def("setConfiguration",
             &abb::egm::EGMControllerInterface::setConfiguration,
             py::arg("configuration"),
             "Update the interface configuration");
}