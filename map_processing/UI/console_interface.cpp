#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "string_utils.h"

namespace map_processing {
    using namespace std;

    void start_console_gui(shared_ptr<unordered_map<size_t, vector<pair<size_t, uint32_t>>>>& distributed_set) {
        string command;
        while (true) {
            getline(cin, command);
            strip(command);
            transform(command.begin(), command.end(), command.begin(), ::toupper);
            if(command == "EXIT") {
                break;
            }if (starts_with(command, "STAT")) {
                size_t station_index = stoi(command.substr(4));
                (*distributed_set)[station_index];
                std::cout << "STAT" << station_index << ":{" << endl;
                for (auto i: (*distributed_set)[station_index]) {
                    std::cout << "\tHOUSE" << i.first << "(distance)" << i.second << ";" << endl;
                }
                cout << "}" << endl;
            }
        }
    }
}