#include "common/kmg_parser.h"

namespace mm_utils {

std::string get_unit_str_(int level) {
    std::string output = " ";
    if (level == 0) {
        // do nothing
    } else if (level == 1) {
        output.append("K");
    } else if (level == 2) {
        output.append("M");
    } else if (level == 3) {
        output.append("G");
    } else if (level == 4) {
        output.append("T");
    } else if (level == 5) {
        output.append("P");
    } else {
        output.append("?");
    }
    return output;
}

std::string get_count_str(int64_t num) {
    return get_disp_str_(num, 1000, 0);
}

std::string get_count_str(int64_t num, int64_t one_k) {
    return get_disp_str_(num, one_k, 0);
}

std::string get_count_str(int64_t num, int64_t one_k, int start_level) {
    return get_disp_str_(num, one_k, start_level);
}

std::string get_byte_str(int64_t num) {
    return get_disp_str_(num, 1024, 0);
}

std::string get_dims_str(const int* nums, int length, int64_t one_k) {
    std::string output;
    for (int i = 0; i < length; ++i) {
        if (output.size() > 0) {
            output.append(", ");
        }
        output.append(get_disp_str_(nums[i], one_k, 0));
    }
    return output;
}

std::string get_disp_str_(int64_t num, int64_t one_k, int start_level) {
    const int64_t half_k = one_k / 2 - 1;
    int level = start_level;
    int64_t curr_num = num;
    while (curr_num > one_k) {
        int64_t new_num = (curr_num + half_k) / one_k;
        double ratio = 1.0 * new_num * one_k / curr_num;
        if (ratio < 0.95 || ratio > 1.05) {
            break;
        }
        curr_num = new_num;
        level += 1;
    }
    return std::to_string(curr_num) + get_unit_str_(level);
}

}
