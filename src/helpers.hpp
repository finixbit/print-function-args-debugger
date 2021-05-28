#ifndef H_HELPERS
#define H_HELPERS

#include <string>


namespace poc {
    namespace helpers {
        std::string long_to_hex_string(long long value);

        template <typename T> 
        std::string to_string(const T& t);

        bool contains_string(
            std::string data, 
            std::string search
        );

        void print_address(std::intptr_t &data, const std::string info);

        void print_address(uint64_t &data, const std::string info);

        void print_address(uint8_t &data, const std::string info);
    }
}

#endif