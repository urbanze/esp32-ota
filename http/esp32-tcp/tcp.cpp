#include "tcp.h"



//===========TCP Client===========
/**
 * @brief Close socket if object are destroyed
 */
TCP_CLIENT::~TCP_CLIENT()
{
    close(s);
}

/**
 * @brief Get socket returned by TCP_SERVER::sv() when client connect.
 * 
 * After client connect in your TCP server, TCP_SERVER::sv() will return and
 * you can R/W data in TCP_CLIENT object.
 * See "How to use in "Github example in README.md
 * 
 * @param [sock]: Socket returned by TCP_SERVER::sv()
 */
void TCP_CLIENT::get_sock(int16_t sock)
{
    s = sock;
}

/**
 * @brief Connect in external TCP server.
 * 
 * @param [*host]: IP String. Eg: "192.168.4.1".
 * @param [port]: Port.
 * 
 * @return [0]: Fail.
 * @return [1]: Sucess.
 */
int8_t TCP_CLIENT::connecto(const char *host, uint16_t port)
{
    int16_t aux;
    uint8_t dns = 0;
    struct sockaddr_in sock;
    struct addrinfo *res, hints;

    //Automatic dns setup if detect A-Z || a-z (trouble with ipv6?!)
    //DNS work even if host is IP Address (number), remove 'ifs (!dns)' in future.
    for (int16_t i = 0; i < strlen(host); i++)
    {
        if (host[i] >= 65 && host[i] <= 90) {dns = 1; break;}
        if (host[i] >= 97 && host[i] <= 122) {dns = 1; break;}
    }

    //Socket
    if (dns) //DNS
    {
        char cport[6] = {0};

        snprintf(cport, 6, "%d", port);
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET,
        hints.ai_socktype = SOCK_STREAM,


        aux = getaddrinfo(host, cport, &hints, &res);
        if (aux != 0 || res == NULL)
        {
            ESP_LOGE(tag, "DNS fail: %d", aux);
            return 0;
        }

        //struct in_addr *addr;
        //addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        //ESP_LOGI(tag, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, IPPROTO_IP);
    }
    else //NO DNS
    {
        sock.sin_addr.s_addr = inet_addr(host);
        sock.sin_family = AF_INET;
        sock.sin_port = htons(port);

        s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    }

    if (s < 0)
    {
        ESP_LOGE(tag, "Fail to create socket [%d]", errno);
        return 0;
    }


    //Connect
    if (dns)
    {
        aux = connect(s, res->ai_addr, res->ai_addrlen);
    }
    else
    {
        aux = connect(s, (struct sockaddr*)&sock, sizeof(sock));
    }

    if (aux != 0)
    {
        //ESP_LOGE(tag, "Fail to connect [%d]", errno);
        close(s);
        return 0;
    }

    return 1;
}

/**
 * @brief Stop (close) TCP connection.
 * 
 * Disconnect any client in your TCP server [returned by TCP_SERVER::sv()].
 * Disconnect you from external server.
 */
void TCP_CLIENT::stop()
{
    close(s);
}

/**
 * @brief Flush (erase) all data available in RX queue.
 */
void TCP_CLIENT::flush()
{
    uint8_t bff = 0;
    int16_t avl = available();
    for (int16_t i = 0; i < avl; i++)
    {
        recv(s, &bff, 1, MSG_DONTWAIT);
    }
}

/**
 * @brief Check if any client/data are connected/avaialble to read in your TCP server.
 * 
 * @return [0]: No Client/data availables.
 * @return [1]: Client/data available.
 */
int8_t TCP_CLIENT::connected()
{
    uint8_t bff = 0;
    recv(s, &bff, 0, MSG_PEEK|MSG_DONTWAIT);
    if (errno == EAGAIN) {return 1;}
    return 0;
}

/**
 * @brief Get how many Bytes are available to read.
 * 
 * @return Bytes available to read.
 */
int16_t TCP_CLIENT::available()
{
    int16_t avl = 0;
    ioctl(s, FIONREAD, &avl);
    return avl;
}

/**
 * @brief Send data over TCP connection.
 * 
 * @param [*text]: Data do send.
 * 
 * @return Data wrote.
 */
int16_t TCP_CLIENT::write(uint8_t *data, uint16_t size)
{
    if (send(s, data, size, 0) < 0)
    {
        ESP_LOGE(tag, "Fail to send [%d]", errno);
        close(s);
        return -1;
    }

    return size;
}

/**
 * @brief Send data over TCP connection.
 * 
 * printf() alias.
 *  
 * @return Data wrote.
 */
int16_t TCP_CLIENT::printf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    int16_t size = vsnprintf(NULL, 0, format, vl)+1;
    va_end(vl);

    char bff[size] = {0};
    vsprintf(bff, format, vl);

    return write((uint8_t*)bff, size);
}

/**
 * @brief Send data over TCP connection.
 * 
 *  
 * @return Data wrote.
 */
int16_t TCP_CLIENT::print(const char *str)
{
    return write((uint8_t*)str, strlen(str));
}

/**
 * @brief Read only one Byte of data available.
 * 
 * @return Data of Byte value readed.
 */
uint8_t TCP_CLIENT::read()
{
    uint8_t bff = 0;
    recv(s, &bff, 1, MSG_DONTWAIT);
    return bff;
}

/**
 * @brief Read [size] Bytes of data available.
 */
void TCP_CLIENT::readBytes(uint8_t *bff, uint16_t size)
{
    //memset(bff, 0, sizeof(size));
    recv(s, bff, size, MSG_DONTWAIT);
}

/**
 * @brief Read [size] Bytes of data available.
 */
void TCP_CLIENT::readBytes(char *bff, uint16_t size)
{
    //memset(bff, 0, sizeof(size));
    recv(s, bff, size, MSG_DONTWAIT);
}



//==============TCP Server==============
/**
 * @brief Close socket if object are destroyed
 */
TCP_SERVER::~TCP_SERVER()
{
    close(s);
}

/**
 * @brief Start TCP Server to listening specific port.
 * 
 * @param [port]: TCP port to listen.
 */
void TCP_SERVER::begin(uint16_t port)
{
    int16_t aux;

    struct sockaddr_in sock;
    sock.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s < 0)
    {
        ESP_LOGE(tag, "Fail to create socket [%d]", errno);
        return;
    }

    aux = bind(s, (struct sockaddr *)&sock, sizeof(sock));
    if (aux != 0)
    {
        ESP_LOGE(tag, "Fail to bind socket [%d]", errno);
        close(s);
        return;
    }


    aux = listen(s, 5);
    if (aux != 0)
    {
        ESP_LOGE(tag, "Fail to listening socket [%d]", errno);
        close(s);
        return;
    }
}

/**
 * @brief Create a instance and wait client connection until [timeout]sec.
 * 
 * This need to be used with TCP_CLIENT::get_sock().
 * When client connect in TCP Server, this function will return immediately and
 * return instance to use in TCP_CLIENT object (read/write/etc).
 * 
 * See "How to use" in Github example (README.md)
 * Attention: This function block current task.
 * 
 * @param [timeout]: Max seconds to wait client connection.
 * 
 * @return Socket instance to use in TCP_CLIENT::get_sock().
 */
int16_t TCP_SERVER::sv(int32_t timeout)
{
    fd_set fds;
    struct timeval stv;
    int16_t c = -1;

    int16_t max = s + 1;
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    stv.tv_sec = timeout;
    stv.tv_usec = 0;

    memset(rmt_ip, 0, sizeof(rmt_ip));
    select(max, &fds, NULL, NULL, &stv);
    if (FD_ISSET(s, &fds))
    {
        struct sockaddr_in src;
        socklen_t src_len = sizeof(src);

        c = accept(s, (struct sockaddr *)&src, &src_len);
        if (c < 0)
        {
            ESP_LOGE(tag, "Fail to accept socket connections [%d]", errno);
            close(s);
        }

        inet_ntop(AF_INET, &src.sin_addr , rmt_ip, sizeof(rmt_ip));
    }

    return c;
}

/**
 * @brief Get source IP from last received packet.
 */
char *TCP_SERVER::remoteIP()
{
    return rmt_ip;
}