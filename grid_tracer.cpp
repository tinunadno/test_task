#include <unordered_map>
#include <memory>
#include "stored_structures.h"
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace map_processing {
    using namespace std;

    static pair<uint32_t, uint32_t> calculate_house_bounds(shared_ptr<SimpleArray> &hs_map, uint32_t i, uint32_t j) {
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

    namespace bg = boost::geometry;
    namespace bg_i = boost::geometry::index;
    using Point = bg::model::point<uint32_t, 2, bg::cs::cartesian>;
    using Value = pair<Point, size_t>;

    using InstancesMap = unordered_map<size_t, vector<pair<size_t, uint32_t>>>;

    shared_ptr<InstancesMap> complete_grid_trace(shared_ptr<SimpleArray> &hs_map) {
        bg_i::rtree <Value, bg_i::quadratic<16>> stations_rtree;
        vector<House> houses;
        size_t station_counter = 0;
        size_t house_counter = 0;
        cout << "STATIONS:" << endl;
        for (uint32_t i = 0; i < hs_map->y_size; i++) {
            for (uint32_t j = 0; j < hs_map->x_size; j++) {
                if (hs_map->get(j, i) == 2) {
                    stations_rtree.insert({{j, i}, station_counter++});
                    std::cout << station_to_string({j, i, station_counter}) << endl;
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

        cout << "HOUSES:" << endl;
        for (auto &i: houses) {
            cout << house_to_string(i) << endl;
            vector<Value> query_result;
            Point query(i.x, i.y);
            stations_rtree.query(bg_i::nearest(query, 1), back_inserter(query_result));

            Station closest_station = {
                    bg::get<0>(query_result[0].first),
                    bg::get<1>(query_result[0].first),
                    query_result[0].second
            };
//            if(distributed_set->find(closest_station.number) == distributed_set->end()){
//                (*distributed_set)[closest_station.number].emplace_back(i.number, 0);
//            }else{
            (*distributed_set)[closest_station.number].emplace_back(i.number, (uint32_t) (sqrt(
                    (i.x - closest_station.x) * (i.x - closest_station.x) +
                    (i.y - closest_station.y) * (i.y - closest_station.y)
            )));
        }
        return distributed_set;
    }
}