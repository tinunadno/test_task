#include <unordered_map>
#include <memory>
#include "stored_structures.h"

namespace map_processing {
    using namespace std;

    static pair<uint32_t, uint32_t> calculate_house_bounds(shared_ptr <SimpleArray> &hs_map, uint32_t i, uint32_t j) {
        uint32_t y_house_size = i;
        uint32_t x_house_size = j;
        while (y_house_size < hs_map->y_size - 1 && (*hs_map).get(x_house_size, y_house_size + 1) != 0) {
            y_house_size++;
        }
        while (y_house_size < hs_map->y_size - 1 && (*hs_map).get(x_house_size + 1, y_house_size) != 0) {
            x_house_size++;
        }
        x_house_size++;
        y_house_size++;
        return {x_house_size, y_house_size};
    }

    using InstancesMap = unordered_map<Station, vector<House>, station_hash>;

    shared_ptr <InstancesMap> complete_grid_trace(shared_ptr <SimpleArray> &hs_map) {
        vector<Station> stations;
        vector<House> houses;
        size_t station_counter = 0;
        size_t house_counter = 0;
        for (uint32_t i = 0; i < hs_map->y_size; i++) {
            for (uint32_t j = 0; j < hs_map->x_size; j++) {
                if (hs_map->get(j, i) == 2) {
                    stations.push_back({j, i, station_counter++});
                }
                if (hs_map->get(j, i) == 1) {
                    auto [x_house_size, y_house_size] = calculate_house_bounds(hs_map, i, j);
                    uint32_t tmp = j;
                    j = x_house_size;
                    uint32_t middle_x_cords = tmp + (x_house_size - tmp) / 2;
                    uint32_t middle_y_cords = i + (y_house_size - i) / 2;
                    houses.push_back(
                            {middle_x_cords, middle_y_cords, x_house_size - tmp, y_house_size - i, house_counter++});
                }
            }
        }
        auto distributed_set = make_shared<InstancesMap>();

        return distributed_set;
    }
}