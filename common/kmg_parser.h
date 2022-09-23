#ifndef __COMMON_KMG_PARSER_H__
#define __COMMON_KMG_PARSER_H__

#include <cstdint>
#include <string>

namespace mm_utils {

std::string get_count_str(int64_t num);
std::string get_count_str(int64_t num, int64_t one_k);
std::string get_count_str(int64_t num, int64_t one_k, int start_level);

std::string get_byte_str(int64_t num);

std::string get_dims_str(const int* nums, int length, int64_t one_k);

std::string get_disp_str_(int64_t num, int64_t one_k, int start_level);
std::string get_unit_str_(int level);

}

#endif
