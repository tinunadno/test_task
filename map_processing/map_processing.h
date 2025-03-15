//
// Created by yura on 3/15/25.
//

#ifndef TEST_TASK_MAP_PROCESSING_H
#define TEST_TASK_MAP_PROCESSING_H

#include "stored_structures/stored_structures.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace map_processing {
    using InstancesMap = unordered_map<size_t, pair<vector<pair<size_t, uint32_t>>, bool>>;
    shared_ptr<SimpleArray> read_file(string &);

    shared_ptr<InstancesMap > complete_grid_trace(shared_ptr<SimpleArray> &);

    void start_console_gui(shared_ptr<InstancesMap> &);
}
#endif //TEST_TASK_MAP_PROCESSING_H
