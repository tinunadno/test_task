#include <memory>
#include <fstream>
#include "stored_structures.h"
#include "map_processing_exceptions.h"

namespace map_processing{
    using namespace std;
    shared_ptr<SimpleArray> read_file(string &file_name) {
        ifstream fileStream;
        fileStream.open(file_name, ios::binary);
        if (!fileStream.is_open()) {
            throw ProcessingException("No such file or directory!");
        }
        uint32_t x_size;
        uint32_t y_size;
        fileStream.read(reinterpret_cast<char *>(&x_size), sizeof(x_size));
        fileStream.read(reinterpret_cast<char *>(&y_size), sizeof(y_size));

        long vector_size = static_cast<long>(x_size) * y_size;

        fileStream.seekg(0, ios::end);
        size_t fileSize = fileStream.tellg();
        fileStream.seekg(sizeof(x_size) + sizeof(y_size), ios::beg);
        fileSize -= sizeof(x_size) + sizeof(y_size);
        if (fileSize < vector_size) {
            throw ProcessingException("File is invalid, less grid, then expected");
        }

        auto buffer = make_shared<SimpleArray>(x_size, y_size);
        fileStream.read(reinterpret_cast<char *>(buffer->grid.data()), vector_size);

        return buffer;
    }
}