#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "../string_utils.h"

namespace map_processing {
    using namespace std;

    using InstancesMap = unordered_map<size_t, pair<vector<pair<size_t, uint32_t>>, bool>>;

    void start_console_gui(shared_ptr<InstancesMap>& distributed_set) {
        string command;
        while (true) {
            getline(cin, command);
            strip(command);
            transform(command.begin(), command.end(), command.begin(), ::toupper);
            if(command == "EXIT") {
                break;
            }if (starts_with(command, "STAT")) {
                size_t station_index;
                try {
                    station_index = stoi(command.substr(4));
                }catch (...){
                    cout << "INVALID COMMAND" << endl;
                    continue;
                }
                if(distributed_set->find(station_index) == (*distributed_set).end()){
                    cout << "NO MATCHING STATIONS FOUND" << endl;
                    continue;
                }
                auto& current_instance = (*distributed_set)[station_index];
                if(!current_instance.second){
                    sort(current_instance.first.begin(), current_instance.first.end(), [](auto& a, auto& b){
                        return a.second > b.second;
                    });
                    current_instance.second = true;
                }
                std::cout << "STAT" << station_index << ":{" << endl;
                for (auto i: current_instance.first) {
                    std::cout << "\tHOUSE" << i.first << "(distance)" << i.second << ";" << endl;
                }
                cout << "}" << endl;
            }else{
                cout << "INVALID COMMAND" << endl;
            }
        }
    }
}