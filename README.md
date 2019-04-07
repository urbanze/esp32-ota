# ESP32 Generic OTA
Languages:	1 - English.
			2 - Portuguese.
            


# 2. Portuguese
* O que é
	* A biblioteca Generic OTA foi desenvolvida especialmente para o microcontrolador ESP32 com suporte de multiplas formas 		para atualizações OTA padrões (não criptografadas) ou criptografadas por AES 128 ECB. Inicialmente desenvolvida baseada na biblioteca WiFi do Arduino 	Core ou Arduino Component com ESP-IDF. Futuramente, haverá versões não dependentes do core Arduino WiFi, ou seja, utilizando apenas a LwIP.

* Objetivos
	* Facilitar ao máximo a implimentação e utilização ao código do usuário.
	* Causar pouco impacto ao processamento do microcontrolador, por isso, foi desenvolvida em uma tarefa do FreeRTOS com baixa 	frequencia em maior parte do tempo, exceto durante o download da atualização.
	* Além das atualizações padrões, quando é enviado apenas o binário pós compilação, a biblioteca também suporta a descriptografia de binários enviados por AES 128 ECB, o que adiciona uma camada relativamente boa de segurança, visto que o binário não ficará mais exposto para qualquer um que baixar sem a chave criptografica.
	* Obter diferentes meios de atualizações OTA padrão e criptografada como:
		* WiFi (TCP, UDP, HTTP).
		* SD.
		* Bluetooth.
		* Serial (UART, SPI, I2C).
		* GSM (SIM 800, SIM 900, A6).
		* LoRa.
		
		
	
* Atenção: Todas formas de atualizações precisam que o ESP32 tenha o esquema de partições com OTA definido nas preferências da placa.


# 2.1 OTA TCP
* Procedimento para atualizações OTA TCP padrões (não criptografadas)
	* Verifique o arquivo de exemplo `ota_default` na pasta `ota_tcp`.
	* As atualizações OTA via TCP são enviadas ao ESP32 na porta de rede 22180.
	* O binário precisa ser enviado sem qualquer byte, header ou itens adicionais, o que causararia falha na atualização por 	integridade corrompida.
	
	* 1. Crie o objeto da biblioteca.
	* 2. Inicie o WiFi em STA ou AP.
	* 3. Inicialize o OTA com a função `ota.init()` apenas uma vez, não coloque em loop.
	* 4. A partir deste momento, a biblioteca já esta monitorando a porta 22180 esperando o binário ser enviado para efetuar o download e update.
	
	
* Procedimento para atualizações OTA TCP criptografadas (AES 128 ECB)
	* Verifique o arquivo de exemplo `ota_crypted` na pasta `ota_tcp`.
	* As atualizações OTA criptografadas via TCP são enviadas ao ESP32 na porta de rede 22180.
	* O binário precisa ser enviando sem qualquer byte, header ou itens adicionais, o que causaria falha na atualização por integridade corrompida.
	* O binário precisa ser enviado criptografado por AES 128 ECB com a mesma chave declarada no código.
	* A chave do AES 128 ECB precisa ter 16 caracteres (letras, números ou caracteres especiais).
	
	* 1. Crie o objeto da biblioteca.
	* 2. Inicie o WiFi em STA ou AP.
	* 3. Inicialize o OTA com a função `ota.init(char key[])` e insira sua chave de 16 caracteres no parâmetro da função. Faça isso apenas uma vez, não coloque em loop.
	* 4. A partir deste momento, a biblioteca já esta monitorando a porta 22180 esperando o binário criptografado ser enviado para efetuar o download e update.
	
