#include <iostream>
#include <memory>
#include <vector>
#include <fstream>

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
        virtual shared_ptr<ProcessingData> process(shared_ptr<ProcessingData>) = 0;

        virtual ~DataProcessor() = default;
    };

    class FinalProcessingUnit {
    public:
        virtual void process(shared_ptr<ProcessingData>) = 0;

        virtual ~FinalProcessingUnit() = default;
    };

    namespace processing_types{
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


        struct House{
            uint32_t x_center;
            uint32_t y_center;
            uint32_t x_size;
            uint32_t y_size;
            size_t house_number;
        };

        struct Station{
            uint32_t x_center;
            uint32_t y_center;
            size_t station_number;
        };

        class HouseStationSet : public ProcessingData {
        public:
            vector<House> houses;
            vector<Station> stations;
        };
    }

    namespace IO {

        using processing_types::TextData;
        using processing_types::HouseStationMap;
        using processing_types::HouseStationSet;

        class HouseStationPrinter: public DataProcessor{
        public:
            HouseStationPrinter() = default;
            shared_ptr<ProcessingData> process(shared_ptr<ProcessingData> processingData) override{
                auto hs_set = dynamic_pointer_cast<HouseStationSet>(processingData);
                if(!hs_set){
                    throw ProcessingDataTypeMissmatch("Types missmatch in HouseStationPrinter: expected HouseStationSet!");
                }
                cout << "Houses <House_Name>: {CORDS: {x, y}; SIZE: {x, y}}" << endl;
                for(auto i : hs_set->houses){
                    cout << "HOUSE" << i.house_number << ": {CORDS: {" << i.x_center << ", " << i.y_center <<"}; SIZE: {" << i.x_size << ", " << i.y_size <<"}}" << endl;
                }
                cout << "Stations <Station_name>: {CORDS: {x, y}}";
                for(auto i : hs_set->stations){
                    cout << "STAT" << i.station_number << ": {CORDS: {" << i.x_center <<", " << i.y_center << "}}" << endl;
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
                        if(hs_map->hs_map[i][j] == 2){
                            hs_set->stations.push_back({j, i, station_counter++});
                        }
                        if(hs_map->hs_map[i][j] == 1){
                            if(i != 0 && hs_map->hs_map[i - 1][j] == 1){
                                while(j < hs_map->x_size - 1 && hs_map->hs_map[i][j + 1] == 1){
                                    j++;
                                }
                            }else{
                                uint32_t y_house_size = i;
                                uint32_t x_house_size = j;
                                while(y_house_size < hs_map->y_size - 1 && hs_map->hs_map[y_house_size + 1][x_house_size] != 0){
                                    y_house_size++;
                                }
                                while(y_house_size < hs_map->y_size - 1 && hs_map->hs_map[y_house_size][x_house_size + 1] != 0){
                                    x_house_size++;
                                }
                                x_house_size++;
                                y_house_size++;
                                uint32_t tmp = j;
                                j = x_house_size;
                                uint32_t middle_x_cords = tmp + (x_house_size - tmp) / 2;
                                uint32_t middle_y_cords = i + (y_house_size - i) / 2;
                                hs_set->houses.push_back({middle_x_cords, middle_y_cords, x_house_size - tmp, y_house_size - i, house_counter++});
                            }
                        }
                    }
                }
                return hs_set;
            }
        };

    }


}

using namespace map_processing;
using namespace map_processing::IO;
using namespace map_processing::processing_core;

int main() {
    auto td = make_shared<TextData>();
    td->text = "/home/yura/Applications/clion/clionProjects/test_task/data.dat";
    auto RF = make_shared<ReadFile>();
    auto mp = dynamic_pointer_cast<HouseStationMap>(RF->process(td));
    auto hss = make_shared<HousesStationTracer>();
    auto data = dynamic_pointer_cast<HouseStationSet>(hss->process(mp));

    for(int i = 0; i< mp->y_size; i++){
        for(int j = 0; j < mp->x_size; j++){
            if(mp->hs_map[i][j] == 0){
                std::cout << "0";
            }
            if(mp->hs_map[i][j] == 1){
                std::cout << "1";
            }
            if(mp->hs_map[i][j] == 2){
                std::cout << "2";
            }
        }
        std::cout << endl;
    }

    auto printer = make_shared<HouseStationPrinter>();
    printer->process(data);
}
