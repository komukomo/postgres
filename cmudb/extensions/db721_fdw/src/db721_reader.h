#pragma once
#include <iostream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <vector>

// clang-format off
extern "C" {
// #include "../../../../src/include/postgres.h"
#include "postgres.h"
#include "utils/builtins.h"
}
#undef vsnprintf
#undef snprintf
#undef vsprintf
#undef sprintf
#undef vfprintf
#undef fprintf
#undef vprintf
#undef printf
#undef gettext
#undef dgettext
#undef ngettext
#undef dngettext
// clang-format on

enum class DataType {
    Float,
    Int,
    Str,
};

typedef struct BlockStats {
    std::string name;
    DataType type ;
    int num;
    int min_len;
    int max_len;
    int min_int;
    int max_int;
    float min_float;
    float max_float;
    char* min_str;
    char* max_str;
} BlockStats;

typedef struct ColumnMeta {
    std::string name;
    DataType type ;
    int start_offset;
    int num_blocks;
    std::vector<BlockStats> *block_stats;
} ColumnMeta;


typedef struct Metadata {
    std::string tablename;
    int max_values_per_block;
    std::vector<ColumnMeta> *columns;
} Metadata;

Metadata read_metadata(FILE *fp);
// void read_column(FILE *fp, Metadata meta, std::string column_name);


class Db721Reader {
public:
    std::vector<char*> data_blocks;
    std::vector<std::string> column_names;
    std::vector<DataType> column_types;
    FILE *fp;
    Metadata meta;
    int current_row;
    int current_block;
    int block_size;
    Db721Reader(FILE *fp);
    void read_block();
    bool read_next(Datum *datum);
    void read_column(int block_idx, int column_idx);
    Datum datum_by_row(int column_idx);
    void display();
};