#include <string>
#include <time.h>
#include "tty_tester.h"

enum { COM_BUFFER_SIZE=32768, };

enum { CRC16_INIT_VAL=0x8006,  };

static uint8_t tx_buffer[COM_BUFFER_SIZE] = {0};
static uint8_t rx_buffer[COM_BUFFER_SIZE] = {0};

/* Function prototypes */


TTY_Tester::TTY_Tester(std::string tty, int baudrate) : m_tty(tty), m_baudrate(baudrate)
{
    m_com = new Serial(m_tty, m_baudrate);

    Serial::en_com_port_ret com_resp = m_com->connect();
    if (com_resp != Serial::RET_OK)
    {
		L_(lerror) << "Failed to connect to tty: " << tty;
        exit(-1);
    }
    
    L_(linfo) << "Connected to " << tty;

    init_buffer(tx_buffer, COM_BUFFER_SIZE);
}

TTY_Tester::~TTY_Tester()
{
    if (m_com != NULL) {
        m_com->disconnect();
        delete m_com;
    }
}

void TTY_Tester::init_buffer(uint8_t * buffer, size_t buffer_size)
{
    for (int i=0; i<buffer_size; i++) {
        buffer[i] = 0xff & i;
    }
    // Calculate CRC
    uint16_t crc = crc16(buffer, buffer_size);
    L_(linfo) << "Initialized buffer with CRC16: 0x" << std::hex << crc;
}

void TTY_Tester::add_test(size_t block_size, size_t n_blocks)
{
    struct test_item item = {block_size, n_blocks, 0};
    m_tests.push_back(item);
    L_(linfo) << "-> Add test: block=" << block_size << ", blocks: " << n_blocks;
}

size_t TTY_Tester::get_num_of_tests()
{
    return(m_tests.size());
}

int TTY_Tester::run_test(size_t index, struct test_item & item)
{
    if (m_tests.size() > index) {
        item = m_tests.at(index);
        
        struct timespec start_time = get_time_ns();

        // m_com->send(tx_buffer, 10);
        /* prepare packet */
        struct packet * out = (struct packet *) &tx_buffer[0];
        out->preamble = PREAMBLE;
        out->length = item.block_size * item.n_blocks;
        out->crc16 = crc16(&tx_buffer[sizeof(out) - 1], out->length);

        for (size_t i = 0; i<item.n_blocks; i++) {
            rx_buffer[0] = rx_buffer[1] = 0;
            m_com->send(&tx_buffer[i], item.block_size);
        }
        m_com->receive_tm(rx_buffer, 2, 100);
        uint16_t * nob = (uint16_t*) &rx_buffer[0];

        struct timespec stop_time = get_time_ns();
        item.time = timespec_diff(start_time, stop_time);

        L_(linfo) << "-> b: " << item.block_size << ", n:" << item.n_blocks << ", nsec: " << item.time
                << ", bytes sent: " << std::dec << *nob;

        return(0);
    }
    return -1;
}

struct timespec TTY_Tester::get_time_ns()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return(t);
}

long TTY_Tester::timespec_diff (struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec * 1000000000.0 + diff.tv_nsec);
}


uint16_t TTY_Tester::crc16(const uint8_t * buffer, uint16_t len)
{
    unsigned short out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if( buffer == NULL )
        return 0;

    while( len > 0 )
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= ( *buffer >> bits_read ) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;

        if( bits_read > 7 )
        {
            bits_read = 0;
            buffer++;
            len--;
        }

        /* Cycle check: */
        if( bit_flag )
            out ^= CRC16_INIT_VAL;

    }

    // item b) "push out" the last 16 bits
    int i;

    for( i = 0; i < 16; ++i )
    {
        bit_flag = out >> 15;
        out <<= 1;

        if( bit_flag )
            out ^= CRC16_INIT_VAL;
    }

    // item c) reverse the bits
    unsigned short crc = 0;
    i = 0x8000;
    int j = 0x0001;

    for( ; i != 0; i >>= 1, j <<= 1 )
    {
        if( i & out )
            crc |= j;
    }

    return crc;
}