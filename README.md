# ESP32 Generic OTA
Languages:	1 - English.
			2 - Portuguese.
            
=====================================================

# 2 Portuguese
* O que é?
	* A biblioteca Generic OTA foi desenvolvida especialmente para o microcontrolador ESP32 com suporte de multiplas formas 		para atualizações OTA padrão ou criptografadas por AES 128. Inicialmente desenvolvida baseada na biblioteca WiFi do Arduino 	Core ou Arduino Component com ESP-IDF. Futuramente, haverá versões não dependentes do core Arduino WiFi, ou seja, utilizando 	apenas a LwIP.

* Objetivos
	* Facilitar ao máximo a implimentação e utilização ao código do usuário.
	* Causar pouco impacto ao processamento do microcontrolador, por isso, foi desenvolvida em uma tarefa do FreeRTOS com baixa 	frequencia em maior parte do tempo, exceto durante o download da atualização.
	* Além das atualizações padrões, quando é enviado apenas o binário pós compilação, a biblioteca também suporta a descriptografia de binários enviados por AES 128 ECB, o que adiciona uma camada relativamente boa de segurança, visto que o binário não ficará mais exposto para qualquer um que baixar sem a chave criptografica.
	* Obter diferentes meios de atualizações OTA padrão e criptografada como:
		* WiFi (TCP, UDP, HTTP).
		* SD.
		* Bluetooth.
		* Serial (UART, SPI, I2C).


