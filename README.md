# ESP32 Generic OTA
Languages:	1 - English.
			2 - Portuguese.
            
===================================================================================================================================

# 2 Portuguese
* O que é?
A biblioteca Generic OTA foi desenvolvida especialmente para o microcontrolador ESP32 com suporte de multiplas formas para atualizações OTA padrão ou criptografadas por AES 128. Inicialmente desenvolvida baseada na biblioteca WiFi do Arduino Core ou Arduino Component com ESP-IDF. Futuramente, haverá versões não dependentes do core Arduino WiFi, ou seja, utilizando apenas a LwIP.

* Objetivos
	* Facilitar ao máximo a implimentação e utilização ao código do usuário
		* Causar pouco impacto ao processamento do microcontrolador, por isso, foi desenvolvida em uma tarefa do FreeRTOS com baixa 	frequencia em maior parte do tempo, exceto durante o download da atualização.



