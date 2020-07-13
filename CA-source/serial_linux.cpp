#include <sys/ioctl.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string>
#include "log.h"
#include "serial_linux.h"


Serial::Serial(std::string port, int baudrate) : m_port(port), m_baudrate(baudrate)
{
    m_fd = -1;
    m_connected = 0;
}

Serial::~Serial()
{
    disconnect();
}

Serial::en_com_port_ret Serial::connect()
{
    return(open_port(m_port.c_str(), m_baudrate, &m_oldopts));
}

Serial::en_com_port_ret Serial::disconnect()
{
    en_com_port_ret resp = RET_ERR_CLOSE;

    tcsetattr(m_fd, TCSANOW, &m_oldopts);
    int err = close(m_fd);
    if (!err) {
        resp = RET_OK;
    }
    m_connected = 0;
    return(resp);
}

size_t Serial::send(const uint8_t * buffer, size_t len)
{
    return(write(m_fd, buffer, len));
}

size_t Serial::receive_tm(uint8_t * buffer, size_t len, int timeout)
{
    struct termios opts;
    tcgetattr(m_fd, &opts);
    opts.c_cc[VTIME] = timeout;
    opts.c_cc[VMIN] = 0;
//    tcflush(m_fd, TCIFLUSH);
//    tcsetattr(m_fd, TCSANOW, &opts);
//    ioctl(m_fd, TCIFLUSH, 0);
    return(read(m_fd, buffer, len));
}

size_t Serial::receive_tm_count(uint8_t * buffer, size_t len, int timeout, int bytes)
{
    if (bytes > 0xff)
    {
        size_t bytes_remain = bytes;
        size_t bytes_received = 0;
        size_t rd_count = 0;
        struct termios opts;

        tcgetattr(m_fd, &opts);
        opts.c_cc[VTIME] = timeout;
        opts.c_cc[VMIN] = 0;
        ioctl(m_fd, TCIFLUSH, 0);
        tcflush(m_fd, TCIFLUSH);
        tcsetattr(m_fd, TCSANOW, &opts);
        while (bytes_remain)
        {
            rd_count = read(m_fd, &buffer[bytes_received], len);
            bytes_received += rd_count;
			L_(ldebug) << "Received: " << bytes_received << "/" << bytes;
            bytes_remain -= rd_count;
			L_(ldebug) << "Remain: " << bytes_remain;
        }
        return(bytes_received);
    }
    else
    return(receive_tm(buffer, len, timeout));
}

size_t Serial::receive(uint8_t * buffer, size_t len)
{
    return(read(m_fd, buffer, len));
};

Serial::en_com_port_ret Serial::open_port(const char *device, int baudrate, struct termios *oldopts)
{
    int unix_speed;
    struct termios opts;

    m_connected = 0;
    unix_speed = baudrate_to_unix(baudrate);

    if (unix_speed < 0)
    {
        return RET_ERR_UNKNOWN_BAUD;
    }

    m_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL);

    if (m_fd < 0) {
        L_(ldebug) << "tty: " << device << ", baud: " << baudrate;
        return RET_ERR_OPEN;
    }

    ioctl(m_fd, TIOCEXCL);

    // no delay
    fcntl(m_fd, F_SETFL, 0);

    /* get previous options */
    tcgetattr(m_fd, &opts);

    /* Save previous options */
    memcpy(oldopts, &opts, sizeof(struct termios));

    /* Set 8,N,1 */
    opts.c_cflag &= ~CSIZE;
    opts.c_cflag &= ~CSTOPB;
    opts.c_cflag &= ~CRTSCTS;
    opts.c_cflag &= ~~(PARENB | PARODD); /* disable parity */
    opts.c_cflag |= CLOCAL | CREAD | CS8;
    opts.c_iflag = IGNPAR;
    opts.c_iflag &= ~(IXON | IXOFF | IXANY);/* disable xon/xoff ctrl */
    opts.c_oflag = 0;
    opts.c_lflag = 0;

    opts.c_cc[VTIME] = 10;
    opts.c_cc[VMIN] = 0;

    cfsetispeed(&opts, unix_speed);
    cfsetospeed(&opts, unix_speed);

    tcflush(m_fd, TCIFLUSH);
    if (tcsetattr(m_fd, TCSANOW, &opts) != 0)
    {
        return RET_ERR_SET_OPTS;
    }
    m_connected = 1;

    return RET_OK;
}

uint8_t Serial::connected(void)
{
    return(m_connected);
}

int Serial::baudrate_to_unix(int speed)
{
    int unix_speed;

    switch (speed)
    {
        case 50: unix_speed = B50; break;
        case 75: unix_speed = B75; break;
        case 110: unix_speed = B110; break;
        case 134: unix_speed = B134; break;
        case 150: unix_speed = B150; break;
        case 300: unix_speed = B300; break;
        case 600: unix_speed = B600; break;
        case 1200: unix_speed = B1200; break;
        case 1800: unix_speed = B1800; break;
        case 2400: unix_speed = B2400; break;
        case 4800: unix_speed = B4800; break;
        case 9600: unix_speed = B9600; break;
        case 19200: unix_speed = B19200; break;
        case 38400: unix_speed = B38400; break;
        case 57600: unix_speed = B57600; break;
        case 115200: unix_speed = B115200; break;
        case 230400: unix_speed = B230400; break;
        case 460800: unix_speed = B460800; break;
        case 500000: unix_speed = B500000; break;
        case 576000: unix_speed = B576000; break;
        case 921600: unix_speed = B921600; break;
        case 1000000: unix_speed = B1000000; break;
        case 1152000: unix_speed = B1152000; break;
        case 1500000: unix_speed = B1500000; break;
        case 2000000: unix_speed = B2000000; break;
        case 2500000: unix_speed = B2500000; break;
        case 3000000: unix_speed = B3000000; break;
        case 3500000: unix_speed = B3500000; break;
        case 4000000: unix_speed = B4000000; break;
        default: unix_speed = -1; break;
    }

    return unix_speed;
}

void Serial::msleep(int ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&ts, NULL);
}
