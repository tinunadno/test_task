#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <set>


using namespace map_processing;
int main() {
    string file_name = "/home/yura/Applications/clion/clionProjects/test_task/grid.dat";
    auto data = read_file(file_name);
    auto complete_set = complete_map_trace(data);
    for(auto& i: *complete_set){
        std::cout << station_to_string(i.first);
        for(auto j : i.second){
            std:: cout << "\t" << house_to_string(j);
        }
        cout << endl;
    }
}
