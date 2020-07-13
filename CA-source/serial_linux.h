/*
 * serial_linux.h
 *
 * The serial RS-232 interface for Linux machines
 *
 *  Created on: Feb 3, 2017
 *      Author: Dimitris Tassopoulos
 *
 * Copyright 2017 MUSIC Group IP Ltd. All rights reserved.
 */

#pragma once

#include <termios.h>

/**
 * @brief A serial/com port Unix object that can read and
 * write data from a serial device
 */
class Serial
{
public:

    enum {RET_CODE_BASE = -20};
    /**
     * @brief Com port return codes
     */
    typedef enum
    {
        RET_OK = 0,
        RET_ERR_UNKNOWN_BAUD = RET_CODE_BASE,
        RET_ERR_OPEN  = RET_CODE_BASE -1,
        RET_ERR_CLOSE  = RET_CODE_BASE -2,
        RET_ERR_SET_OPTS = RET_CODE_BASE -3,
    } en_com_port_ret;

    /**
     * @brief Create a Unix com port object
     * @param port The system com port path (e.g. /dev/hermes)
     * @param baudrate The com port baud rate speed
     */
    Serial(std::string port, int baudrate);

    /**
     * @brief Destroy the COM object
     */
    virtual ~Serial();

    /**
     * @brief Open COM port
     */
    en_com_port_ret connect();

    /**
     * @brief Close COM port
     */
    en_com_port_ret disconnect();

    /**
     * @brief Send buffer to the COM port
     * @param[in] buffer The buffer to send
     * @param[in] len The length of the buffer
     * @return size_t The number of bytes written
     */
    size_t send(const uint8_t * buffer, size_t len);

    /**
     * @brief Receive data from COM port
     * @param[out] buffer The buffer to store the received data
     * @param[in] len The size of the input buffer
     * @return size_t The number of the received bytes
     */
    size_t receive(uint8_t * buffer, size_t len);

    /**
     * @brief Receive data from COM port with a timeout
     * @param[out] buffer The buffer to store the received data
     * @param[in] len The size of the input buffer
     * @param[in] timeout The timeout value (1/10 secs)
     * @return size_t The number of the received bytes
     */
    size_t receive_tm(uint8_t * buffer, size_t len, int timeout);

    /**
     * @brief Receive data with specific length from COM port using a timeout
     * @param[out] buffer The buffer to store the received data
     * @param[in] len The size of the input buffer
     * @param[in] timeout The timeout value (1/10 secs)
     * @param[in] int The number of bytes to wait for receive
     * @return size_t The number of the received bytes
     */
    size_t receive_tm_count(uint8_t * buffer, size_t len, int timeout, int bytes);

    uint8_t connected(void);

private:
    /**
     * @brief Open a unix com port
     * @param[in] device The device name (e.g. /dev/ttyACMx or /dev/hermes)
     * @param[in] speed The com port baud rate speed
     * @param[in] oldopts The previous com port options
     * @return[in] int Return a COM_RET_* response
     */
    en_com_port_ret open_port(const char *device, int speed, struct termios *oldopts);

    /**
     * @brief Convert baud rate speed to unix speed
     * @param[in] speed The baudrate value (e.g. 115200)
     * @return int The unix equivalent (e.g. B115200 = 0010011)
     */
    int baudrate_to_unix(int speed);

    /**
     * @brief Sleep for given ms
     * @param[in] ms The number of millisecs to sleep
     */
    void msleep(int ms);
    std::string m_port;
    int m_baudrate;
    uint8_t m_connected;

    int m_fd;
    struct termios m_oldopts;
};