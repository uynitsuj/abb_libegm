// src/udp.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "abb_libegm/egm_udp_server.h"

namespace py = pybind11;

// Trampoline class for AbstractUDPServerInterface
class PyAbstractUDPServer : public abb::egm::AbstractUDPServerInterface {
public:
    // Inherit constructors
    using abb::egm::AbstractUDPServerInterface::AbstractUDPServerInterface;

    // Override pure virtual function
    const std::string& callback(const abb::egm::UDPServerData& data) override {
        PYBIND11_OVERRIDE_PURE(
            const std::string&,            // Return type
            abb::egm::AbstractUDPServerInterface,  // Parent class
            callback,                      // Function name
            data                          // Arguments
        );
        static std::string empty;
        return empty;
    }

    // Public wrapper for the private callback
    const std::string& public_callback(const abb::egm::UDPServerData& data) {
        return this->callback(data);
    }
};

void bind_udp(py::module& m) {
    // Wrap UDPServerData
    py::class_<abb::egm::UDPServerData>(m, "UDPServerData")
        .def(py::init<>())
        .def_readwrite("port_number", &abb::egm::UDPServerData::port_number)
        .def_readwrite("bytes_transferred", &abb::egm::UDPServerData::bytes_transferred);

    // Wrap AbstractUDPServerInterface as pure virtual interface
    py::class_<abb::egm::AbstractUDPServerInterface, PyAbstractUDPServer>(m, "AbstractUDPServerInterface")
        .def(py::init<>())
        .def("callback", 
             [](PyAbstractUDPServer& self, const abb::egm::UDPServerData& data) {
                 return self.public_callback(data);
             },
             py::arg("data"),
             "Process received data and return reply");

    // Wrap UDPServer
    py::class_<abb::egm::UDPServer>(m, "UDPServer")
        .def(py::init<boost::asio::io_service&, unsigned short, abb::egm::AbstractUDPServerInterface*>(),
             py::arg("io_service"),
             py::arg("port_number"),
             py::arg("interface"))
        .def("isInitialized", &abb::egm::UDPServer::isInitialized,
             "Check if the server was successfully initialized");
}