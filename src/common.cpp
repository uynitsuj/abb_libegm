// src/common.cpp

#include <pybind11/pybind11.h>
#include "abb_libegm/egm_common.h"
#include "abb_libegm/egm_base_interface.h"

namespace py = pybind11;

void bind_common(py::module& m) {
    // Bind enums
    py::enum_<abb::egm::RobotAxes>(m, "RobotAxes")
        .value("None", abb::egm::RobotAxes::None)
        .value("Six", abb::egm::RobotAxes::Six)
        .value("Seven", abb::egm::RobotAxes::Seven)
        .export_values();

    py::enum_<abb::egm::EGMModes>(m, "EGMModes")
        .value("EGMJoint", abb::egm::EGMModes::EGMJoint)
        .value("EGMPose", abb::egm::EGMModes::EGMPose)
        .export_values();

    // Bind BaseConfiguration
    py::class_<abb::egm::BaseConfiguration>(m, "BaseConfiguration")
        .def(py::init<>())
        .def_readwrite("axes", &abb::egm::BaseConfiguration::axes)
        .def_readwrite("use_demo_outputs", &abb::egm::BaseConfiguration::use_demo_outputs)
        .def_readwrite("use_velocity_outputs", &abb::egm::BaseConfiguration::use_velocity_outputs)
        .def_readwrite("use_logging", &abb::egm::BaseConfiguration::use_logging)
        .def_readwrite("max_logging_duration", &abb::egm::BaseConfiguration::max_logging_duration);

    // Bind EGMBaseInterface
    py::class_<abb::egm::EGMBaseInterface>(m, "EGMBaseInterface")
        .def(py::init<boost::asio::io_service&, const unsigned short, const abb::egm::BaseConfiguration&>(),
            py::arg("io_service"),
            py::arg("port_number"),
            py::arg("configuration") = abb::egm::BaseConfiguration())
        .def("isInitialized", &abb::egm::EGMBaseInterface::isInitialized)
        .def("isConnected", &abb::egm::EGMBaseInterface::isConnected)
        .def("getStatus", &abb::egm::EGMBaseInterface::getStatus)
        .def("getConfiguration", &abb::egm::EGMBaseInterface::getConfiguration)
        .def("setConfiguration", &abb::egm::EGMBaseInterface::setConfiguration);

    // Constants submodule
    py::module_ constants = m.def_submodule("Constants", "ABB LibEGM Constants");
    
    // Add constants
    constants.attr("LOWEST_SAMPLE_TIME") = abb::egm::Constants::RobotController::LOWEST_SAMPLE_TIME;
    constants.attr("DEFAULT_PORT_NUMBER") = abb::egm::Constants::RobotController::DEFAULT_PORT_NUMBER;
    constants.attr("DEFAULT_NUMBER_OF_ROBOT_JOINTS") = abb::egm::Constants::RobotController::DEFAULT_NUMBER_OF_ROBOT_JOINTS;
    // ... other constants
}