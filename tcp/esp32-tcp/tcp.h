#ifndef tcp_H
#define tcp_H

#include <esp_err.h>
#include <esp_log.h>

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>



/**
  	@file tcp.h
	@author Jose Morais

	Class to use TCP Client & Server (BSD Sockets)


	MIT License

	Copyright (c) 2020 José Morais

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

class TCP_CLIENT
{
	private:
		const char tag[11] = "TCP_CLIENT";
		int s = 0;

	public:
		~TCP_CLIENT();
		void get_sock(int16_t sock);

		int8_t connecto(const char *host, uint16_t port);
		void stop();

		int8_t connected();
		int16_t available();
		void flush();

		int16_t write(uint8_t *data, uint16_t size);
		int16_t printf(const char *format, ...);
		int16_t print(const char *str);

		uint8_t read();
		void readBytes(uint8_t *bff, uint16_t size);
		void readBytes(char *bff, uint16_t size);
		
};

class TCP_SERVER
{
	private:
		const char tag[11] = "TCP_SERVER";
		int s = 0;
		char rmt_ip[16] = {0};

	public:
		~TCP_SERVER();

		void begin(uint16_t port);
		int16_t sv(int32_t timeout);
		char *remoteIP();

};


#endif