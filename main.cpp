#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#ifdef __unix__
#define UNIX_USER

#include <ncurses.h>

#endif


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

    namespace math_utils {
        auto distance_func = [](uint32_t x1, uint32_t y1, uint32_t x2, u_int32_t y2) {
            return (float) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        };
    }

    namespace processing_types {
        using math_utils::distance_func;

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

        float calculate_distance_between_hs(House &h, Station &s) {
            return distance_func(h.x_center, h.y_center, s.x_center, s.y_center);
        }

        class HouseStationSet : public ProcessingData {
        public:
            HouseStationSet() = default;

            vector<House> houses;
            vector<Station> stations;
            uint32_t max_x{};
            uint32_t max_y{};
        };

        class HouseStationTable : public ProcessingData {
        public:
            unordered_map<size_t, size_t> house_station_table;
            unordered_map<size_t, House> house_table;
            unordered_map<size_t, Station> station_table;
            size_t max_x{};
            size_t max_y{};
        };
    }

    namespace tiny_database {
        using processing_types::HouseStationTable;
        using processing_types::house_to_string;
        using processing_types::station_to_string;
        using string_utils::strip;
        using processing_types::calculate_distance_between_hs;

        class CommandProcessor {
        public:
            explicit CommandProcessor(shared_ptr<HouseStationTable> &hs_table)
                    : hs_table(hs_table) {
                command_map["SELECT"] = &CommandProcessor::handle_select;
                command_map["SHOW"] = &CommandProcessor::handle_show;
                command_map["STATTRACE"] = &CommandProcessor::handle_stat_trace;
                command_map["HOUSEREL"] = &CommandProcessor::handle_house_rel;
                cool_mode_command_map["STATTRACE"] = &CommandProcessor::handle_station_trace_for_cool_mode;
                cool_mode_command_map["HOUSEREL"] = &CommandProcessor::handle_select_house_for_cool_mode;
                command_descriptions.emplace_back("SELECT",
                                                  "[syntax: SELECT <HOUSES/STATIONS> <index>] show house or station with certain index");
                command_descriptions.emplace_back("SHOW",
                                                  "[syntax: SHOW <HOUSES/STATIONS>] show all instances of house or station");
                command_descriptions.emplace_back("STATTRACE",
                                                  "[syntax: STATTRACE <index>] showing all the houses, connected to a certain station");
                command_descriptions.emplace_back("HOUSEREL",
                                                  "[syntax: HOUSEREL <index/ALL>] showing all houses and stations they are connected");
            }

            void print_command_descriptions() {
                cout << "AVAILABLE COMMANDS:" << endl;
                for (const auto &i: command_descriptions) {
                    cout << i.first << ": " << i.second << endl;
                }
            }

            string process_command(const string &command) {
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

            pair<size_t, vector<size_t>> process_command_for_cool_mode(string &command) {
                string stripped_command = command;
                strip(stripped_command);

                size_t split_index = find_split_index(stripped_command);
                if (split_index == string::npos) {
                    return {};
                }

                string command_name = stripped_command.substr(0, split_index);
                string command_rest = stripped_command.substr(split_index + 1);

                transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);

                auto it = cool_mode_command_map.find(command_name);
                if (it != cool_mode_command_map.end()) {
                    return (this->*(it->second))(command_rest);
                }

                return {};
            }

        private:
            pair<size_t, vector<size_t>> handle_select_house_for_cool_mode(string &args) {
                return house_rel_by_index_for_cool_mode(args);
            }

            pair<size_t, vector<size_t>> handle_station_trace_for_cool_mode(string &args) {
                transform(args.begin(), args.end(), args.begin(), ::toupper);
                return station_trace_for_cool_mode(stoi(args));
            }

            string handle_select(string &args) {
                size_t split_index = find_split_index(args);
                if (split_index == string::npos) {
                    return "INVALID SELECT COMMAND, type help to see all available commands";
                }
                string table_name = args.substr(0, split_index);
                transform(table_name.begin(), table_name.end(), table_name.begin(), ::toupper);
                size_t index = stoi(args.substr(split_index + 1));
                return select_command(table_name, index);
            }

            string handle_show(string &args) {
                transform(args.begin(), args.end(), args.begin(), ::toupper);
                return show_command(args);
            }

            string handle_stat_trace(string &args) {
                transform(args.begin(), args.end(), args.begin(), ::toupper);
                return station_trace(stoi(args));
            }

            string handle_house_rel(string &args) {
                if (args == "ALL") {
                    return house_relations(args);
                } else {
                    return house_rel_by_index(args);
                }
            }

            static size_t find_split_index(const string &str) {
                const string attempt_characters = " \t";
                for (char c: attempt_characters) {
                    size_t index = str.find(c);
                    if (index != string::npos) {
                        return index;
                    }
                }
                return string::npos;
            }

            pair<size_t, vector<size_t>> house_rel_by_index_for_cool_mode(string &args) {
                size_t house_index = stoi(args);
                auto it = hs_table->house_table.find(house_index);
                if (it == hs_table->house_table.end()) {
                    return {};
                }
                pair<size_t, vector<size_t>> ret;
                size_t station_index = hs_table->house_station_table[house_index];
                ret.first = station_index;
                vector<size_t> house = {house_index};
                ret.second = house;
                return ret;
            }

            string select_command(const string &table_name, size_t index) {
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

            string show_command(const string &table_name) {
                string ret;
                if (table_name == "HOUSE") {
                    for (const auto &i: hs_table->house_station_table) {
                        auto current_house = hs_table->house_table[i.first];
                        ret += house_to_string(current_house) + "\n";
                    }
                } else if (table_name == "STATION") {
                    for (const auto &i: hs_table->station_table) {
                        auto current_station = i.second;
                        ret += station_to_string(current_station) + "\n";
                    }
                } else {
                    ret = "NO MATCHING TABLE FOUND!";
                }
                return ret;
            }

            pair<size_t, vector<size_t>> station_trace_for_cool_mode(size_t station_index) {
                if (hs_table->station_table.find(station_index) == hs_table->station_table.end()) {
                    return {};
                }
                auto it = station_trace_cache.find(station_index);
                if (it == station_trace_cache.end()) {
                    vector<pair<size_t, float>> houses_belongs_to_station;
                    for (const auto &i: hs_table->house_station_table) {
                        if (i.second == station_index) {
                            houses_belongs_to_station.emplace_back(i.first, calculate_distance_between_hs(
                                    hs_table->house_table[i.first], hs_table->station_table[i.second]));
                        }
                    }
                    sort(houses_belongs_to_station.begin(), houses_belongs_to_station.end(), [](auto a, auto b) {
                        return a.second > b.second;
                    });
                    station_trace_cache.insert({station_index, houses_belongs_to_station});
                }
                pair<size_t, vector<size_t>> ret;
                ret.first = station_index;
                vector<size_t> houses;
                for (auto i: station_trace_cache[station_index]) {
                    houses.push_back(i.first);
                }
                ret.second = houses;
                return ret;
            }

            string station_trace(size_t station_index) {
                if (hs_table->station_table.find(station_index) == hs_table->station_table.end()) {
                    return "NO MATCHING STATIONS FOUND";
                }
                auto it = station_trace_cache.find(station_index);
                if (it == station_trace_cache.end()) {
                    vector<pair<size_t, float>> houses_belongs_to_station;
                    for (const auto &i: hs_table->house_station_table) {
                        if (i.second == station_index) {
                            houses_belongs_to_station.emplace_back(i.first, calculate_distance_between_hs(
                                    hs_table->house_table[i.first], hs_table->station_table[i.second]));
                        }
                    }
                    sort(houses_belongs_to_station.begin(), houses_belongs_to_station.end(), [](auto a, auto b) {
                        return a.second > b.second;
                    });
                    station_trace_cache.insert({station_index, houses_belongs_to_station});
                }
                string ret = station_to_string(hs_table->station_table[station_index]);
                string houses;
                size_t counter = 0;
                for (auto i: station_trace_cache[station_index]) {
                    counter++;
                    houses += "\t" + house_to_string(hs_table->house_table[i.first]) + " (distance: " +
                              to_string(i.second) + ")\n";
                }
                if (houses.empty()) {
                    return ret + " -> NO HOUSES FOUND";
                }
                return ret + " (TOTAL " + to_string(counter) + ") ->{\n" + houses + "}";
            }

            string house_relations(string &args) {
                string ret;
                for (auto i: hs_table->house_station_table) {
                    ret += house_to_string(hs_table->house_table[i.first]) + " <- " +
                           station_to_string(hs_table->station_table[i.second]) + "\n";
                }
                return ret;
            }

            string house_rel_by_index(string &args) {
                size_t house_index = stoi(args);
                auto it = hs_table->house_table.find(house_index);
                if (it == hs_table->house_table.end()) {
                    return "NO MATCHING HOUSES FOUND";
                }
                string ret;
                size_t station_index = hs_table->house_station_table[house_index];
                ret += house_to_string(hs_table->house_table[house_index]) + " -> " +
                       station_to_string(hs_table->station_table[station_index]);
                return ret;
            }

            using CommandHandler = string (CommandProcessor::*)(string &);
            using CoolModeCommandHandler = pair<size_t, vector<size_t>> (CommandProcessor::*)(string &);
            unordered_map<string, CommandHandler> command_map;
            vector<pair<string, string>> command_descriptions;
            unordered_map<string, CoolModeCommandHandler> cool_mode_command_map;

            shared_ptr<HouseStationTable> hs_table;
            unordered_map<size_t, vector<pair<size_t, float>>> station_trace_cache;
        };
    }

    namespace UI {
        using processing_types::HouseStationTable;
        using processing_types::House;
        using processing_types::Station;
        using processing_types::house_to_string;
        using processing_types::station_to_string;
        using tiny_database::CommandProcessor;
        using string_utils::strip;

        class ConsoleUI : public FinalProcessingUnit {
        public:
            ConsoleUI() = default;

            void process(shared_ptr<ProcessingData> data) override {
                cout << "type *help* to start, *exit* to leave\n";
                auto hs_table = dynamic_pointer_cast<HouseStationTable>(data);
                if (!hs_table) {
                    throw ProcessingDataTypeMissmatch("Type missmatch in ConsoleUI: expected HouseStationTable!");
                }
                auto commandProcessor = new CommandProcessor(hs_table);
                pair<size_t, vector<size_t>> last_station_trace;
                bool was_trace = false;
                string current_command;
                string last_message;
                while (true) {
#ifdef UNIX_USER
                    if (cool_mode_enabled) {
                        erase();
                        draw_all_instances_lines(hs_table);
                        if (was_trace) {
                            Station current_station = hs_table->station_table[last_station_trace.first];
                            for (auto i: last_station_trace.second) {
                                House current_house = hs_table->house_table[i];
                                draw_instance_lines(current_house, current_station, hs_table->max_x, hs_table->max_y,
                                                    '@');
                                refresh();
                            }
                        }
                        subscribe_all_instances(hs_table);
                        mvprintw(0, 0, "%s", last_message.c_str());
                        refresh();

                        current_command = cool_mode_get_command();
                        strip(current_command);
                        transform(current_command.begin(), current_command.end(), current_command.begin(), ::toupper);

                        if (current_command == "EXIT") {
                            endwin();
                            break;
                        }
                        if (current_command == "COOLMODE") {
                            endwin();
                            cool_mode_enabled = !cool_mode_enabled;
                            continue;
                        }
                        if (current_command == "HELP") {
                            last_message = "WARNING: exit cool mode before typing *help*\n";
                            continue;
                        }

                        pair<size_t, vector<size_t>> station_traces = commandProcessor->process_command_for_cool_mode(
                                current_command);
                        if (station_traces.second.empty()) {
                            last_message = "INVALID COMMAND";
                            continue;
                        }

                        last_station_trace = station_traces;
                        was_trace = true;
                        last_message = "last command: '" + current_command + "'";
                    }
#endif
                    if (!cool_mode_enabled) {
                        getline(cin, current_command);
                        strip(current_command);
                        transform(current_command.begin(), current_command.end(), current_command.begin(), ::toupper);
                        if (current_command == "EXIT") {
                            break;
                        }
                        if (current_command == "HELP") {
                            commandProcessor->print_command_descriptions();
                            cout
                                    << "COOLMODE: [syntax COOLMODE] switch to simple map with houses and stations, you should resize your console for better resolution"
                                    << endl;
                        } else if (current_command == "COOLMODE") {

#ifdef UNIX_USER
                            cout << "WARNING!" << endl << "in coolmode only STATTRACE and HOUSEREL are available!"
                                 << endl << "continue[y/n]";
                            string temp;
                            getline(cin, temp);
                            strip(temp);
                            if (temp != "y") {
                                continue;
                            }
                            cool_mode_enabled = !cool_mode_enabled;
                            initscr();
                            cbreak();
                            noecho();
                            keypad(stdscr, TRUE);
#else
                            std::cout << "WARNING: to use cool mode, you have to install ncurce" << endl << "type 'sudo apt-get install libncurses5-dev libncursesw5-dev' to do this\n";
#endif
                        } else {
                            cout << commandProcessor->process_command(current_command) << endl;
                        }
                    }
                }
                delete commandProcessor;
            }

        private:

#ifdef UNIX_USER

            static void draw_all_instances_lines(shared_ptr<HouseStationTable> &hs_table) {
                for (auto i: hs_table->house_station_table) {
                    House current_house = hs_table->house_table[i.first];
                    Station current_station = hs_table->station_table[i.second];
                    draw_instance_lines(current_house, current_station, hs_table->max_x, hs_table->max_y, '#');
                }
            }

            static void
            draw_instance_lines(House &current_house, Station &current_station, uint32_t max_x, uint32_t max_y,
                                char c) {
                float s_x = (float) current_station.x_center / (float) max_x;
                float s_y = (float) current_station.y_center / (float) max_y;
                float h_x = (float) current_house.x_center / (float) max_x;
                float h_y = (float) current_house.y_center / (float) max_y;
                cool_mode_draw_line(s_x, s_y, h_x, h_y, c);
            }

            static void subscribe_all_instances(shared_ptr<HouseStationTable> &hs_table) {
                for (auto i: hs_table->house_table) {
                    House ch = i.second;
                    float h_x = (hs_table->max_x != 0) ? static_cast<float>(ch.x_center) /
                                                         static_cast<float>(hs_table->max_x) : 0;
                    float h_y = (hs_table->max_y != 0) ? static_cast<float>(ch.y_center) /
                                                         static_cast<float>(hs_table->max_y) : 0;
                    cool_mode_draw_centered_text(h_x, h_y, "HOUSE" + to_string(ch.house_number));
                }

                for (auto i: hs_table->station_table) {
                    Station cs = i.second;
                    float s_x = (hs_table->max_x != 0) ? static_cast<float>(cs.x_center) /
                                                         static_cast<float>(hs_table->max_x) : 0;
                    float s_y = (hs_table->max_y != 0) ? static_cast<float>(cs.y_center) /
                                                         static_cast<float>(hs_table->max_y) : 0;
                    cool_mode_draw_centered_text(s_x, s_y, "STATION" + to_string(cs.station_number));
                }
            }

            static string cool_mode_get_command() {
                int maxY, maxX;
                getmaxyx(stdscr, maxY, maxX);

                move(maxY - 1, 0);
                clrtoeol();
                printw("input_your_command> ");
                refresh();

                string input;
                int ch;
                while ((ch = getch()) != '\n') {
                    if (ch == KEY_BACKSPACE || ch == 127) {
                        if (!input.empty()) {
                            input.pop_back();
                            addch('\b');
                            addch(' ');
                            addch('\b');
                        }
                    } else {
                        input.push_back((char) ch);
                        addch(ch);
                    }
                    refresh();
                }

                return input;
            }

            static void cool_mode_draw_centered_text(float x_norm, float y_norm, const std::string &text) {
                int maxY, maxX;
                getmaxyx(stdscr, maxY, maxX);

                int x = static_cast<int>(x_norm * (float) maxX);
                int y = static_cast<int>(y_norm * (float) (maxY - 2)) + 1;

                int len = static_cast<int>(text.length());
                int startX = x - len / 2;

                if (startX >= 0 && startX + len <= maxX && y >= 0 && y < maxY) {
                    mvprintw(y, startX, "%s", text.c_str());
                }
            }

            static void cool_mode_draw_line(float x_1, float y_1, float x_2, float y_2, char ch) {
                int maxY, maxX;
                getmaxyx(stdscr, maxY, maxX);

                int x1 = static_cast<int>((float) maxX * x_1);
                int y1 = static_cast<int>((float) (maxY - 2) * y_1) + 1;
                int x2 = static_cast<int>((float) maxX * x_2);
                int y2 = static_cast<int>((float) (maxY - 2) * y_2) + 1;

                int dx = std::abs(x2 - x1);
                int dy = std::abs(y2 - y1);
                int sx = (x1 < x2) ? 1 : -1;
                int sy = (y1 < y2) ? 1 : -1;
                int err = dx - dy;

                while (true) {
                    if (x1 >= 0 && x1 < maxX && y1 >= 1 && y1 < maxY) {
                        mvaddch(y1, x1, ch);
                    }

                    if (x1 == x2 && y1 == y2) break;

                    int e2 = 2 * err;
                    if (e2 > -dy) {
                        err -= dy;
                        x1 += sx;
                    }
                    if (e2 < dx) {
                        err += dx;
                        y1 += sy;
                    }
                }
            }

#endif
            bool cool_mode_enabled = false;
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
                hs_set->max_x = hs_map->x_size;
                hs_set->max_y = hs_map->y_size;
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
                        if (first_time) {
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
                hs_table->max_x = hs_set->max_x;
                hs_table->max_y = hs_set->max_y;
                return hs_table;
            }
        };

    }

    namespace pipeline {
        using processing_types::TextData;
        using namespace IO;
        using namespace UI;
        using namespace processing_core;
        using namespace tiny_database;

        class PipeLine {
        public:
            explicit PipeLine(const vector<shared_ptr<DataProcessor>> &dp, shared_ptr<FinalProcessingUnit> &pl_ending)
                    : processors(dp), pipe_line_ending(pl_ending) {}

            void initiate_pipe_line(const shared_ptr<ProcessingData> &init_data) {
                shared_ptr<ProcessingData> last_return = init_data;
                while (true) {
                    try {
                        last_return = this->process_next(last_return);
                        if (last_return == nullptr) {
                            break;
                        }
                    } catch (ProcessingDataTypeMissmatch &e) {
                        cout << e.what();
                    } catch (ProcessingException &e) {
                        cout << e.what();
                    }
                }
            }

            shared_ptr<ProcessingData> process_next(shared_ptr<ProcessingData> &pd) {
                if (pipe_line_counter < processors.size()) {
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

        void start_map_processing(string &file_name) {
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
