#include <iostream>
#include <fstream>

#include "map_processing.h"

using namespace std;
using namespace map_processing;
int main() {
    string file_name = "/home/yura/Applications/clion/clionProjects/test_task/data.dat";
    auto file = read_file(file_name);
    auto data = complete_grid_trace(file);
    start_console_gui(data);
}
