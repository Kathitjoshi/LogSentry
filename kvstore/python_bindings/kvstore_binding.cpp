#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "lsm_engine.h"

namespace py = pybind11;

PYBIND11_MODULE(kvstore, m) {
    m.doc() = "Python bindings for custom LSM-tree KV store";

    // LSMEngine::Stats struct
    py::class_<kvstore::LSMEngine::Stats>(m, "Stats")
        .def_readonly("memtable_size", &kvstore::LSMEngine::Stats::memtable_size)
        .def_readonly("num_sstables", &kvstore::LSMEngine::Stats::num_sstables)
        .def_readonly("total_disk_usage", &kvstore::LSMEngine::Stats::total_disk_usage)
        .def_readonly("num_keys_approx", &kvstore::LSMEngine::Stats::num_keys_approx);

    // LSMEngine class
    py::class_<kvstore::LSMEngine>(m, "LSMEngine")
        .def(py::init<const std::string&>(),
             py::arg("db_path"),
             "Create or open a database at the given path")
        
        .def("put", &kvstore::LSMEngine::put,
             py::arg("key"), py::arg("value"),
             "Insert or update a key-value pair")
        
        .def("get", [](kvstore::LSMEngine& self, const std::string& key) -> py::object {
            std::string value;
            if (self.get(key, value)) {
                return py::cast(value);
            }
            return py::none();
        }, py::arg("key"),
           "Get value by key, returns None if not found")
        
        .def("remove", &kvstore::LSMEngine::remove,
             py::arg("key"),
             "Delete a key")
        
        .def("compact", &kvstore::LSMEngine::compact,
             "Manually trigger compaction")
        
        .def("get_stats", &kvstore::LSMEngine::get_stats,
             "Get database statistics")
        
        .def("close", &kvstore::LSMEngine::close,
             "Close the database and flush all data");
}
