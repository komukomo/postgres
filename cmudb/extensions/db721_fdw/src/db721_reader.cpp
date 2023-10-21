#include "db721_reader.h"

using json = nlohmann::json;

#define STRING_SIZE 32


ColumnMeta get_column_meta(Metadata *meta, std::string column_name) {
    for (auto it : *(meta->columns)) {
        if (it.name == column_name) {
            return it;
        }
    }
}


int get_size(DataType typ) {
    if (typ == DataType::Float) {
        return 4;
    } else if (typ == DataType::Int) {
        return 4;
    } else if (typ == DataType::Str) {
        return STRING_SIZE;
    }
    // TODO: else Error
}


class Db721Reader {
    public:
    std::vector<char*> data_blocks;
    std::vector<std::string> column_names = {"farm_name", "min_age_weeks", "max_age_weeks"};
    std::vector<DataType> column_types = {DataType::Str, DataType::Float, DataType::Float};
    FILE *fp;
    Metadata meta;
    int current_idx = 0;

    void read_block() {
        for (int i = 0; i < column_names.size(); i++) {
            read_column(column_names[i], 0 ,i);
        }
    }

    void read_column(std::string column_name, int block_idx, int column_idx) {
        auto column_meta = get_column_meta(&meta, column_name);
        fseek(fp, column_meta.start_offset, SEEK_SET);
        int datanum = column_meta.block_stats[block_idx].data()->num;
        int type_size = get_size(column_meta.type);
        int size = sizeof(char) * type_size * datanum;
        data_blocks.push_back((char *)malloc(size));
        fread(data_blocks[column_idx], type_size, datanum, fp);
    }

    void datum_by_row(Datum &datum, int column_idx) {
        int type_size = get_size(column_types[column_idx]);
        int offset = current_idx * type_size;
        if (column_types[column_idx] == DataType::Float) {
            float4 val = 0;
            memcpy(&val, data_blocks[0] + offset, type_size);
            datum = Float4GetDatum(val);
        } else if (column_types[column_idx] == DataType::Int) {
            int val = 0;
            memcpy(&val, data_blocks[0] + offset, type_size);
            datum = Int32GetDatum(val);
        } else if (column_types[column_idx] == DataType::Str) {
            char val[STRING_SIZE+1] = {};
            memcpy(&val, data_blocks[0] + offset, type_size);
            datum = CStringGetDatum(val);
        }
    }

    void display() {
        char data[6][STRING_SIZE];
        memcpy(data, data_blocks[0], 6*STRING_SIZE);
        for (auto d: data) {
            std::cout << d << std::endl;
        }

        {
            float4 data[6];
            memcpy(data, data_blocks[1], 6*sizeof(float4));
            for (auto d: data) {
                std::cout << d << std::endl;
            }
        }
        {
            float4 data[6];
            memcpy(data, data_blocks[2], 6*sizeof(float4));
            for (auto d: data) {
                std::cout << d << std::endl;
            }
        }

    }
};



Metadata read_metadata(FILE *fp) {
    int metaSize;
    // fread(&filesize, sizeof(int), 1, fp);
    int metaSizeByte = sizeof(int);
    fseek(fp, -metaSizeByte, SEEK_END);
    fread(&metaSize, metaSizeByte, 1, fp);

    char *metaBuf;
    metaBuf = (char *) malloc(metaSize + 1);
    fseek(fp, -metaSizeByte-metaSize, SEEK_END);
    fgets(metaBuf, metaSize + 1, fp);

    // std::cout << metaSize;
    std::cout << metaBuf << std::endl;

    json data = json::parse(metaBuf);
    std::vector<ColumnMeta> *column_metas = new std::vector<ColumnMeta>;
    for (auto it = data["Columns"].begin(); it != data["Columns"].end(); it++) {
        ColumnMeta column_meta = {};
        column_meta.name = it.key();
        column_meta.num_blocks = it.value()["num_blocks"].get<int>();
        column_meta.start_offset = it.value()["start_offset"].get<int>();
        auto typ = it.value()["type"].get<std::string>();
        DataType t;
        if (typ == "str") {
            t = DataType::Str;
        } else if (typ == "float") {
            t = DataType::Float;
        } else if (typ == "int") {
            t = DataType::Int;
        } else {
            // TODO: Error
        }
        auto bs = it.value()["block_stats"];
        auto block_stats_vec = new std::vector<BlockStats>;
        for (auto st = bs.begin(); st != bs.end(); st++) {
            BlockStats stats = {};
            stats.num = st.value()["num"].get<int>();
            block_stats_vec->push_back(stats);
        }
        column_meta.type = t;

        column_meta.block_stats = block_stats_vec;
        column_metas->push_back(column_meta);
    };
    auto table = data["Table"].get<std::string>();
    auto max_values_per_block = data["Max Values Per Block"].get<int>();

    Metadata meta = {
        table,
        max_values_per_block,
        column_metas,
    };

    return meta;
}


int main() {
    char *filepath = "/tmp/testdata/data-farms.db721";
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        std::cout << "File not found\n";
        return 1;
    }
    auto meta = read_metadata(fp);
    auto reader = new Db721Reader();
    reader->meta = meta;
    reader->fp = fp;
    reader->read_block();

    reader->display();
    // read_column(fp, meta, "farm_name");
}