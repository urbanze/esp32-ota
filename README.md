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

# 2.0 Instalando a biblioteca
* 1. Baixe o .ZIP da biblioteca.
* ![AI0](https://user-images.githubusercontent.com/29230962/56461440-8c7a3780-6389-11e9-8861-259fe28dd3dd.png)
* 2. Na Arduino IDE, vá em (Sketch > Incluir Biblioteca > Adicionar biblioteca .ZIP) e selecione o arquivo baixado.
* ![AI1](https://user-images.githubusercontent.com/29230962/56461552-49b95f00-638b-11e9-8e45-2d67130a721f.png)
* 3. Na Arduino IDE, vá em (Sketch > Incluir Biblioteca), procure pela biblioteca e clique para inclui-la.
* ![AI2](https://user-images.githubusercontent.com/29230962/56461557-605fb600-638b-11e9-87ea-fd9ac2c67297.png)



# 2.1 OTA TCP
* Indicado para uso em aplicativos ou scripts de Windows, Android, Linux, MAC e etc que façam a transferência de arquivos diretamente para o `socket` sem adição de qualquer byte a mais, como NETCAT no linux.
* Permite outros ESP's enviarem uma atualização para o ESP à ser atualizado, como por exemplo um ESP gateway que pode atualizar todos seus Slaves (escravos)!
* Costuma ser o segundo método mais rápido, abaixo apenas do UDP.
* Permite apenas (por enquanto) o upload de novos APPS, futuramente será adicionado opções de rollback.


* Procedimento para atualizações OTA TCP padrões (não criptografadas)
	* Arduino IDE: Verifique o arquivo de exemplo `tcp_default.ino` na pasta `examples/ota_tcp`.
	* ESP-IDF: Verifique o arquivo de exemplo `idf_tcp_default.c` na pasta `examples/ota_tcp`.
	* É aconselhavel manter o DEBUG_LEVEL em `INFO` para mais informações sobre a biblioteca.
	* As atualizações OTA via TCP são enviadas ao ESP32 na porta de rede 22180.
	* O binário precisa ser enviado sem qualquer byte, header ou itens adicionais, o que causararia falha na atualização por 	integridade corrompida.
	
	* 1. Crie o objeto da biblioteca.
	* 2. Inicie o WiFi em STA ou AP.
	* 3. Inicialize o OTA com o método `.init()` apenas uma vez, não coloque em loop.
	* 4. A partir deste momento, a biblioteca já esta monitorando a porta 22180 esperando o binário ser enviado para efetuar o download e update.
	
	
* Procedimento para atualizações OTA TCP criptografadas (AES 128 ECB)
	* Arduino IDE: Verifique o arquivo de exemplo `tcp_crypted.ino` na pasta `examples/ota_tcp`.
	* ESP-IDF: Verifique o arquivo de exemplo `idf_tcp_crypted.c` na pasta `examples/ota_tcp`.
	* É aconselhavel manter o DEBUG_LEVEL em `INFO` para mais informações sobre a biblioteca.
	* As atualizações OTA criptografadas via TCP são enviadas ao ESP32 na porta de rede 22180.
	* O binário precisa ser enviando sem qualquer byte, header ou itens adicionais, o que causaria falha na atualização por integridade corrompida.
	* O binário precisa ser enviado criptografado por AES 128 ECB com a mesma chave declarada no código.
	* A chave do AES 128 ECB precisa ter 16 caracteres (letras, números ou caracteres especiais).
	
	* 1. Crie o objeto da biblioteca.
	* 2. Inicie o WiFi em STA ou AP.
	* 3. Inicialize o OTA com o método `.init(char key[])` e insira sua chave de 16 caracteres no parâmetro da função. Faça isso apenas uma vez, não coloque em loop.
	* 4. A partir deste momento, a biblioteca já esta monitorando a porta 22180 esperando o binário criptografado ser enviado para efetuar o download e update.
	
# 2.2 OTA HTTP
* Indicado para uso cotidiano, onde a praticidade e simplicidade governam. Mostra algumas informações úteis do dispositivo e APP atual, bastando conectar ao IP do dispositivo na porta 8080.
* Permite retornar (rollback) versões de APPS gravados no ESP32, como o último OTA ou Factory APP (gravado via Serial pelo PC).

* Procedimento para atualizações OTA HTTP padrões (não criptografadas)
	* Arduino IDE: Verifique o arquivo de exemplo `http_default.ino` na pasta `examples/ota_http`.
	* ESP-IDF: Verifique o arquivo de exemplo `idf_http_default.c` na pasta `examples/ota_http`.
	* É aconselhavel manter o DEBUG_LEVEL em `INFO` para mais informações sobre a biblioteca.
	* As atualizações OTA via HTTP são feitas conectando o navegador ao IP do dispositivo e porta 8080, como por exemplo "192.168.4.1:8080".
	* O binário precisa ser enviado sem qualquer byte, header ou itens adicionais, o que causararia falha na atualização por 	integridade corrompida.
	
	* 1. Crie o objeto da biblioteca.
	* 2. Inicie o WiFi em STA ou AP.
	* 3. Inicialize o OTA com o método `.init()` apenas uma vez, não coloque em loop.
	* 4. A partir deste momento, a biblioteca já criou a página HTML para você acessar e efetuar novos uploads, rollback e etc.
