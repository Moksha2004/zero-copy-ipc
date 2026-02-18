#include<pybind11/pybind11.h>
#include<pybind11/numpy.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#include "zcp_ipc.hpp"

namespace py=pybind11;

class PyReader {
    int fd;
    SharedMemoryLayout* layout;



public:
    PyReader(){
        fd=shm_open(SHM_NAME,O_RDWR,0666);
        if(fd==-1) throw std::runtime_error("Writer not running");
        layout=static_cast<SharedMemoryLayout*>(mmap(0,sizeof(SharedMemoryLayout),PROT_READ,MAP_SHARED,fd,0));
        if(layout==MAP_FAILED){
            close(fd);
            throw std::runtime_error("mmap failed");
        }
    }
    //Destructor
    ~PyReader(){
        if(layout) munmap(layout,sizeof(SharedMemoryLayout));
        if(fd==-1) close(fd);
    }
    py::array_t<Market> get_buffer(){
        return py::array_t<Market>(
            {RING_BUFFER_SIZE}, //1024 items
            {sizeof(Market)}, //jump 16 bytes
            layout->buffer, //this is teh pointer
            py::cast(this)
        );
    }
    uint64_t get_head_pos(){
        return layout->control.head.load(std::memory_order_relaxed);
    }
};

PYBIND11_MODULE(zpc,m){
    PYBIND11_NUMPY_DTYPE(Market,symbol_id,price,volume);
    py::class_<PyReader>(m,"Reader")
        .def(py::init<>())
        .def("get_head_pos",&PyReader::get_head_pos)
        .def("get_buffer",&PyReader::get_buffer);
}