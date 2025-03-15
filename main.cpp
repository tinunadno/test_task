#include <iostream>

#include "map_processing/map_processing.h"

using namespace map_processing;

int main(int argc, char* argv[]) {
    if(argc != 2){
        std::cerr << "NO SOURCE FILE PATH WAS GIVEN" << std::endl;
        std::cerr << "type ./test_task 'path_to_source_file' to process it" << std::endl;
        return 1;
    }
    std::string file_name = argv[1];
    auto file = read_file(file_name);
    auto data = complete_grid_trace(file);
    start_console_gui(data);
}
