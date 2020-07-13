#pragma once

#include <vector>
#include "log.h"
#include "serial_linux.h"

class TTY_Tester {
public:
    enum { PREAMBLE=0xABCD, };
    struct test_item {
        size_t block_size;
        size_t n_blocks;
        long time;
    };
    #pragma pack(1)
    struct packet {
        uint16_t preamble;
        uint16_t length;
        uint16_t crc16;
    };

    TTY_Tester(std::string tty, int baudrate);
    virtual ~TTY_Tester();

    void add_test(size_t block_size, size_t n_blocks);
    size_t get_num_of_tests();
    int run_test(size_t index, struct test_item &item);

private:
    Serial * m_com;
    std::string m_tty;
    int m_baudrate;
    std::vector<struct test_item> m_tests;

    void init_buffer(uint8_t * buffer, size_t buffer_size);
    struct timespec get_time_ns();
    long timespec_diff (struct timespec t1, struct timespec t2);
    uint16_t crc16(const uint8_t * buffer, uint16_t len);
};