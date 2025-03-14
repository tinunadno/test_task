#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace map_processing {
    using namespace std;

    class ProcessingDataTypeMissmatch : public exception {
    public:
        explicit ProcessingDataTypeMissmatch(const char *message_) {
            this->message = message_;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return this->message;
        }

    private:
        const char *message;
    };

    class ProcessingException : public exception {
    public:
        explicit ProcessingException(const char *message_) {
            this->message = message_;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return message;
        }

    private:
        const char *message;
    };

    class ProcessingData {
    public:
        virtual ~ProcessingData() = default;
    };

    class DataProcessor {
    public:
        DataProcessor() = default;
        virtual shared_ptr<ProcessingData> process(shared_ptr<ProcessingData>) = 0;

        virtual ~DataProcessor() = default;
    };

    class FinalProcessingUnit {
    public:
        virtual void process(shared_ptr<ProcessingData>) = 0;

        virtual ~FinalProcessingUnit() = default;
    };

    namespace string_utils {
        void strip(string &str) {
            if ((int) str.length() != 0) {
                auto w = string(" ");
                auto n = string("\n");
                auto r = string("\t");
                auto t = string("\r");
                auto v = string(1, str.front());
                while ((v == w) || (v == t) || (v == r) || (v == n)) {
                    str.erase(str.begin());
                    v = string(1, str.front());
                }
                v = string(1, str.back());
                while ((v == w) || (v == t) || (v == r) || (v == n)) {
                    str.erase(str.end() - 1);
                    v = string(1, str.back());
                }
            }
        }
    }

    namespace processing_types {
        class TextData : public ProcessingData {
        public:
            string text;
        };

        class HouseStationMap : public ProcessingData {
        public:
            HouseStationMap(uint32_t x_size, uint32_t y_size) : x_size(x_size), y_size(y_size) {
                hs_map.resize(y_size, vector<u_char>(x_size));
            }

            uint32_t x_size;
            uint32_t y_size;
            vector<vector<uint8_t>> hs_map;
        };


        struct House {
            uint32_t x_center;
            uint32_t y_center;
            uint32_t x_size;
            uint32_t y_size;
            size_t house_number;
        };

        string house_to_string(House &current_house) {
            return "HOUSE" + to_string(current_house.house_number) + ": {CORDS: {" + to_string(current_house.x_center) +
                   ", " +
                   to_string(current_house.y_center) + "}; SIZE: {" + to_string(current_house.x_size) + ", " +
                   to_string(current_house.y_size) + "}}";
        }

        struct Station {
            uint32_t x_center;
            uint32_t y_center;
            size_t station_number;
        };

        string station_to_string(Station &current_station) {
            return "STAT" + to_string(current_station.station_number) + ": {CORDS: {" +
                   to_string(current_station.x_center) + ", " +
                   to_string(current_station.y_center) + "}}";
        }

        class HouseStationSet : public ProcessingData {
        public:
            HouseStationSet() = default;

            vector<House> houses;
            vector<Station> stations;
        };

        class HouseStationTable : public ProcessingData {
        public:
            unordered_map<size_t, size_t> house_station_table;
            unordered_map<size_t, House> house_table;
            unordered_map<size_t, Station> station_table;
        };
    }

    namespace tiny_database {
        using processing_types::HouseStationTable;
        using processing_types::house_to_string;
        using processing_types::station_to_string;
        using namespace string_utils;

        class CommandProcessor {
        public:
            explicit CommandProcessor(shared_ptr<HouseStationTable>& hs_table)
                    : hs_table(hs_table) {
                command_map["SELECT"] = &CommandProcessor::handle_select;
                command_map["SHOW"] = &CommandProcessor::handle_show;
                command_map["STATTRACE"] = &CommandProcessor::handle_stat_trace;
                command_map["HOUSEREL"] = &CommandProcessor::handle_house_rel;
                command_descriptions.emplace_back("SELECT", "[syntax: SELECT <HOUSES/STATIONS> <index>] show house or station with certain index");
                command_descriptions.emplace_back("SHOW", "[syntax: SHOW <HOUSES/STATIONS>] show all instances of house or station");
                command_descriptions.emplace_back("STATTRACE", "[syntax: STATTRACE <index>] showing all the houses, connected to a certain station");
                command_descriptions.emplace_back("HOUSEREL", "[syntax: HOUSEREL <index/ALL>] showing all houses and stations they are connected");
            }

            void print_command_descriptions(){
                cout << "AVAILABLE COMMANDS:" << endl;
                for(const auto& i: command_descriptions){
                    cout << i.first << ": " << i.second << endl;
                }
            }

            string process_command(const string& command) {
                string stripped_command = command;
                strip(stripped_command);

                size_t split_index = find_split_index(stripped_command);
                if (split_index == string::npos) {
                    return "INVALID COMMAND, type help to see all available commands";
                }

                string command_name = stripped_command.substr(0, split_index);
                string command_rest = stripped_command.substr(split_index + 1);

                transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

                auto it = command_map.find(command_name);
                if (it != command_map.end()) {
                    return (this->*(it->second))(command_rest);
                }

                return "INVALID COMMAND, type help to see all available commands";
            }

        private:
            string handle_select(string& args) {
                size_t split_index = find_split_index(args);
                if (split_index == string::npos) {
                    return "INVALID SELECT COMMAND, type help to see all available commands";
                }
                string table_name = args.substr(0, split_index);
                transform(table_name.begin(), table_name.end(), table_name.begin(), ::toupper);
                size_t index = stoi(args.substr(split_index + 1));
                return select_command(table_name, index);
            }

            string handle_show(string& args) {
                transform(args.begin(), args.end(), args.begin(), ::toupper);
                return show_command(args);
            }

            string handle_stat_trace(string& args) {
                transform(args.begin(), args.end(), args.begin(), ::toupper);
                return station_trace(stoi(args));
            }

            string handle_house_rel(string& args){
                if(args == "ALL"){
                    return house_relations(args);
                }else{
                    return house_rel_by_index(args);
                }
            }

            static size_t find_split_index(const string& str) {
                const string attempt_characters = " \t";
                for (char c : attempt_characters) {
                    size_t index = str.find(c);
                    if (index != string::npos) {
                        return index;
                    }
                }
                return string::npos;
            }

            string select_command(const string& table_name, size_t index) {
                if (table_name == "HOUSE") {
                    auto it = hs_table->house_table.find(index);
                    if (it == hs_table->house_table.end()) {
                        return "NO MATCHING HOUSES FOUND";
                    }
                    return house_to_string(hs_table->house_table[index]);
                } else if (table_name == "STATION") {
                    auto it = hs_table->station_table.find(index);
                    if (it == hs_table->station_table.end()) {
                        return "NO MATCHING STATIONS FOUND";
                    }
                    return station_to_string(hs_table->station_table[index]);
                }
                return "NO MATCHING TABLE FOUND!";
            }

            string show_command(const string& table_name) {
                string ret;
                if (table_name == "HOUSE") {
                    for (const auto& i : hs_table->house_station_table) {
                        auto current_house = hs_table->house_table[i.first];
                        ret += house_to_string(current_house)+"\n";
                    }
                } else if (table_name == "STATION") {
                    for (const auto& i : hs_table->station_table) {
                        auto current_station = i.second;
                        ret += station_to_string(current_station)+"\n";
                    }
                } else {
                    ret = "NO MATCHING TABLE FOUND!";
                }
                return ret;
            }

            string station_trace(size_t station_index) {
                if (hs_table->station_table.find(station_index) == hs_table->station_table.end()){
                    return "NO MATCHING STATIONS FOUND";
                }
                auto it = station_trace_cache.find(station_index);
                if (it == station_trace_cache.end()) {
                    vector<size_t> houses_belongs_to_station;
                    for (const auto& i : hs_table->house_station_table) {
                        if (i.second == station_index) {
                            houses_belongs_to_station.push_back(i.first);
                        }
                    }
                    station_trace_cache.insert({station_index, houses_belongs_to_station});
                }
                string ret = station_to_string(hs_table->station_table[station_index]);
                string houses;
                size_t counter = 0;
                for (auto i : station_trace_cache[station_index]) {
                    counter++;
                    houses += "\t" + house_to_string(hs_table->house_table[i]) + "\n";
                }
                if (houses.empty()) {
                    return ret + " -> NO HOUSES FOUND";
                }
                return ret + " (TOTAL " + to_string(counter) + ") ->{\n" + houses + "}";
            }

            string house_relations(string& args){
                string ret;
                for(auto i: hs_table->house_station_table){
                    ret += house_to_string(hs_table->house_table[i.first]) + " <- " + station_to_string(hs_table->station_table[i.second]) + "\n";
                }
                return ret;
            }
            string house_rel_by_index(string& args){
                size_t house_index = stoi(args);
                auto it = hs_table->house_table.find(house_index);
                if(it == hs_table->house_table.end()){
                    return "NO MATCHING HOUSES FOUND";
                }
                string ret;
                size_t station_index = hs_table->house_station_table[house_index];
                ret += house_to_string(hs_table->house_table[house_index]) + " -> " + station_to_string(hs_table->station_table[station_index]);
                return ret;
            }

            using CommandHandler = string (CommandProcessor::*)(string&);
            unordered_map<string, CommandHandler> command_map;
            vector<pair<string, string>> command_descriptions;

            shared_ptr<HouseStationTable> hs_table;
            unordered_map<size_t, vector<size_t>> station_trace_cache;
        };
    }

    namespace UI{
        using processing_types::HouseStationTable;
        using tiny_database::CommandProcessor;
        using string_utils::strip;

        class ConsoleUI : public FinalProcessingUnit {
        public:
            ConsoleUI() = default;
            void process(shared_ptr<ProcessingData> data) override{
                cout << "type *help* to start\n";
                auto hs_table = dynamic_pointer_cast<HouseStationTable>(data);
                if(!hs_table){
                    throw ProcessingDataTypeMissmatch("Type missmatch in ConsoleUI: expected HouseStationTable!");
                }
                auto commandProcessor = new CommandProcessor(hs_table);
                string current_command;
                while(true){
                    getline(cin, current_command);
                    strip(current_command);
                    transform(current_command.begin(), current_command.end(), current_command.begin(), ::toupper);
                    if(current_command == "EXIT"){
                        break;
                    }
                    if(current_command == "HELP"){
                        commandProcessor->print_command_descriptions();
                    }else {
                         cout << commandProcessor->process_command(current_command) << endl;
                    }
                }
                delete commandProcessor;
            }
        };
    }

    namespace IO {

        using processing_types::TextData;
        using processing_types::HouseStationMap;
        using processing_types::HouseStationSet;
        using processing_types::house_to_string;
        using processing_types::station_to_string;

        class HouseStationPrinter : public DataProcessor {
        public:
            HouseStationPrinter() = default;

            shared_ptr<ProcessingData> process(shared_ptr<ProcessingData> processingData) override {
                auto hs_set = dynamic_pointer_cast<HouseStationSet>(processingData);
                if (!hs_set) {
                    throw ProcessingDataTypeMissmatch(
                            "Types missmatch in HouseStationPrinter: expected HouseStationSet!");
                }
                cout << "Houses <House_Name>: {CORDS: {x, y}; SIZE: {x, y}}" << endl;
                for (auto i: hs_set->houses) {
                    cout << house_to_string(i) << endl;
                }
                cout << "Stations <Station_name>: {CORDS: {x, y}}";
                for (auto i: hs_set->stations) {
                    cout << station_to_string(i) << endl;
                }
                return hs_set;
            }
        };

        class ReadFile : public DataProcessor {
        public:
            explicit ReadFile() = default;

            shared_ptr<ProcessingData> process(shared_ptr<ProcessingData> processingData) override {
                auto file_name = dynamic_pointer_cast<TextData>(processingData);
                if (!file_name) {
                    throw ProcessingDataTypeMissmatch("Data missmatch in ReadFile, expected TextData!");
                }
                ifstream fileStream;
                fileStream.open(file_name->text, ios::binary);
                if (!fileStream.is_open()) {
                    throw ProcessingException("No such file or directory!");
                }
                uint32_t x_size;
                uint32_t y_size;
                fileStream.read(reinterpret_cast<char *>(&x_size), sizeof(x_size));
                fileStream.read(reinterpret_cast<char *>(&y_size), sizeof(y_size));

                long vector_size = static_cast<long>(x_size) * y_size;

                fileStream.seekg(0, ios::end);
                size_t fileSize = fileStream.tellg();
                fileStream.seekg(sizeof(x_size) + sizeof(y_size), ios::beg);

                if (fileSize < vector_size) {
                    throw ProcessingException("File is invalid, less data, then expected");
                }

                vector<uint8_t> temp_buffer(vector_size);
                fileStream.read(reinterpret_cast<char *>(temp_buffer.data()), vector_size);

                auto hs_map = make_shared<HouseStationMap>(x_size, y_size);

                for (uint32_t i = 0; i < y_size; i++) {
                    for (uint32_t j = 0; j < x_size; j++) {
                        hs_map->hs_map[i][j] = temp_buffer[i * x_size + j];
                    }
                }
                return hs_map;
            }
        };
    }

    namespace processing_core {
        using processing_types::HouseStationMap;
        using processing_types::HouseStationSet;
        using processing_types::HouseStationTable;
        using processing_types::Station;

        class HousesStationTracer : public DataProcessor {
        public:
            shared_ptr<ProcessingData> process(shared_ptr<ProcessingData> pd) override {
                auto hs_map = dynamic_pointer_cast<HouseStationMap>(pd);
                if (!hs_map) {
                    throw ProcessingDataTypeMissmatch("Data types missmatch: expected HouseStationMap in HSSearch!");
                }
                size_t house_counter = 0;
                size_t station_counter = 0;
                auto hs_set = make_shared<HouseStationSet>();
                for (uint32_t i = 0; i < hs_map->y_size; i++) {
                    for (uint32_t j = 0; j < hs_map->x_size; j++) {
                        if (hs_map->hs_map[i][j] == 2) {
                            hs_set->stations.push_back({j, i, station_counter++});
                        }
                        if (hs_map->hs_map[i][j] == 1) {
                            if (i != 0 && hs_map->hs_map[i - 1][j] == 1) {
                                while (j < hs_map->x_size - 1 && hs_map->hs_map[i][j + 1] == 1) {
                                    j++;
                                }
                            } else {
                                uint32_t y_house_size = i;
                                uint32_t x_house_size = j;
                                while (y_house_size < hs_map->y_size - 1 &&
                                       hs_map->hs_map[y_house_size + 1][x_house_size] != 0) {
                                    y_house_size++;
                                }
                                while (y_house_size < hs_map->y_size - 1 &&
                                       hs_map->hs_map[y_house_size][x_house_size + 1] != 0) {
                                    x_house_size++;
                                }
                                x_house_size++;
                                y_house_size++;
                                uint32_t tmp = j;
                                j = x_house_size;
                                uint32_t middle_x_cords = tmp + (x_house_size - tmp) / 2;
                                uint32_t middle_y_cords = i + (y_house_size - i) / 2;
                                hs_set->houses.push_back(
                                        {middle_x_cords, middle_y_cords, x_house_size - tmp, y_house_size - i,
                                         house_counter++});
                            }
                        }
                    }
                }
                return hs_set;
            }
        };

        class HouseStationSetProcessor : public DataProcessor {
        public:
            HouseStationSetProcessor() = default;

            shared_ptr<ProcessingData> process(shared_ptr<ProcessingData> data) override {
                auto hs_set = dynamic_pointer_cast<HouseStationSet>(data);
                if (!hs_set) {
                    throw ProcessingDataTypeMissmatch(
                            "Data missmatch in HouseStationSetProcessor: expected HouseStationSet");
                }
                auto hs_table = make_shared<HouseStationTable>();
                auto distance_func = [](uint32_t x1, uint32_t y1, uint32_t x2, u_int32_t y2) {
                    return (float) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
                };
                bool first_time = true;
                for (auto i: hs_set->houses) {
                    float min_distance = 100000000000.0f;
                    Station min_distance_station = {0};
                    for (auto j: hs_set->stations) {
                        if(first_time){
                            hs_table->station_table.insert({j.station_number, j});
                        }
                        float current_distance = distance_func(i.x_center, i.y_center, j.x_center, j.y_center);
                        if (current_distance < min_distance) {
                            min_distance = current_distance;
                            min_distance_station = j;
                        }
                    }
                    hs_table->house_table.insert({i.house_number, i});
                    first_time = false;
                    if (min_distance != -1) {
                        hs_table->house_station_table.insert({i.house_number, min_distance_station.station_number});
                    }
                }
                return hs_table;
            }
        };

    }

    namespace pipeline{
        using processing_types::TextData;
        using namespace IO;
        using namespace UI;
        using namespace processing_core;
        using namespace tiny_database;
        class PipeLine{
        public:
            explicit PipeLine(const vector<shared_ptr<DataProcessor>>& dp, shared_ptr<FinalProcessingUnit>& pl_ending): processors(dp), pipe_line_ending(pl_ending){}
            void initiate_pipe_line(const shared_ptr<ProcessingData>& init_data){
                shared_ptr<ProcessingData> last_return = init_data;
                while(true) {
                    try {
                        last_return = this->process_next(last_return);
                        if(last_return == nullptr){
                            break;
                        }
                    } catch (ProcessingDataTypeMissmatch &e) {
                        cout << e.what();
                    } catch (ProcessingException &e) {
                        cout << e.what();
                    }
                }
            }
            shared_ptr<ProcessingData> process_next(shared_ptr<ProcessingData>& pd){
                if(pipe_line_counter < processors.size()){
                    return (processors[pipe_line_counter++])->process(pd);
                }
                pipe_line_ending->process(pd);
                return nullptr;
            }
        private:
            vector<shared_ptr<DataProcessor>> processors;
            shared_ptr<FinalProcessingUnit> pipe_line_ending;
            size_t pipe_line_counter = 0;
        };

        void start_map_processing(string& file_name){
            auto td = make_shared<TextData>();
            td->text = file_name;
            vector<shared_ptr<DataProcessor>> pd = {
                    make_shared<ReadFile>(),
                    make_shared<HousesStationTracer>(),
                    make_shared<HouseStationSetProcessor>()
            };
            auto concole_UI = dynamic_pointer_cast<FinalProcessingUnit>(make_shared<ConsoleUI>());
            auto pl = new PipeLine(pd, concole_UI);
            pl->initiate_pipe_line(td);
            delete pl;
        }
    }

}

using map_processing::pipeline::start_map_processing;

int main() {
    std::string file_name = "/home/yura/Applications/clion/clionProjects/test_task/data.dat";
    start_map_processing(file_name);
}
