#include "stored_structures.h"

namespace map_processing{

    string house_to_string(House &current_house) {
        return "HOUSE" + to_string(current_house.number) + ": {CORDS: {" + to_string(current_house.x) +
               ", " +
               to_string(current_house.y) + "}; SIZE: {" + to_string(current_house.x_size) + ", " +
               to_string(current_house.y_size) + "}}";
    }

    string station_to_string(const Station &current_station) {
        return "STAT" + to_string(current_station.number) + ": {CORDS: {" +
               to_string(current_station.x) + ", " +
               to_string(current_station.y) + "}}";
    }

}