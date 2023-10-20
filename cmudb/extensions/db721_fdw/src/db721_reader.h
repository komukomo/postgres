#pragma once
#include <iostream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <vector>

// clang-format off
extern "C" {
#include "../../../../src/include/postgres.h"
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
void read_column(FILE *fp, Metadata meta, std::string column_name);