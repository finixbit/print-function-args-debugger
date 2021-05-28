#include <string>
#include <cctype>
#include <sstream>
#include <inttypes.h> /* PRIx64 */
#include "helpers.hpp"


std::string poc::helpers::long_to_hex_string(long long value) {
    if(value == 0)
        return std::string("*0x0");

    char pointer_value_at_register[512];
    sprintf(pointer_value_at_register, "0x%llx", value);
    return std::string(pointer_value_at_register);
}

template <typename T> 
std::string poc::helpers::to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

bool poc::helpers::contains_string(std::string data, std::string search) {
    if (data.find(search) != std::string::npos) {
        return true;
    }
    return false;
}

void poc::helpers::print_address(std::intptr_t &data, const std::string info) {
    printf("0x%016" PRIx64 "\t(%s)\n", data, info.c_str());
}

void poc::helpers::print_address(uint64_t &data, const std::string info) {
    printf("0x%016" PRIx64 "\t(%s)\n", data, info.c_str());
}

void poc::helpers::print_address(uint8_t &data, const std::string info) {
    printf("0x%016" PRIx64 "\t(%s)\n", data, info.c_str());
}
