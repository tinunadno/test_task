//
// Created by yura on 3/15/25.
//

#ifndef TEST_TASK_STORED_STRUCTURES_H
#define TEST_TASK_STORED_STRUCTURES_H

#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class SimpleArray {
public:
    SimpleArray(uint32_t x, uint32_t y) : x_size(x), y_size(y), grid(x * y) {}

    uint8_t get(uint32_t x, uint32_t y) const noexcept {
        return grid[x * x_size + y];
    }

    vector<uint8_t> grid;
    uint32_t x_size;
    uint32_t y_size;
};

struct House {
    uint32_t x, y;
    uint32_t x_size, y_size;
    size_t number;
};

string house_to_string(House &current_house) {
    return "HOUSE" + to_string(current_house.number) + ": {CORDS: {" + to_string(current_house.x) +
           ", " +
           to_string(current_house.y) + "}; SIZE: {" + to_string(current_house.x_size) + ", " +
           to_string(current_house.y_size) + "}}";
}

struct Station {
    uint32_t x, y;
    size_t number;

    bool operator==(const Station &other_one) const {
        return number == other_one.number;
    }
};

struct station_hash {
    size_t operator()(const Station &key) const {
        return hash<int>{}(key.number);
    }
};


string station_to_string(const Station &current_station) {
    return "STAT" + to_string(current_station.number) + ": {CORDS: {" +
           to_string(current_station.x) + ", " +
           to_string(current_station.y) + "}}";
}

#endif //TEST_TASK_STORED_STRUCTURES_H
