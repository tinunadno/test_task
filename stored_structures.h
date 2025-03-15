//
// Created by yura on 3/15/25.
//

#ifndef TEST_TASK_STORED_STRUCTURES_H
#define TEST_TASK_STORED_STRUCTURES_H

#include <cstdint>
#include <vector>
#include <string>
namespace map_processing {
    using namespace std;

    class SimpleArray {
    public:
        SimpleArray(uint32_t x, uint32_t y) : x_size(x), y_size(y), grid(x * y) {}

        uint8_t get(uint32_t x, uint32_t y) const noexcept {
            return grid[y * x_size + x];
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

    string house_to_string(House &current_house);

    struct Station {
        uint32_t x, y;
        size_t number;
    };

    string station_to_string(const Station &current_station);
}
#endif //TEST_TASK_STORED_STRUCTURES_H
