# ESP32 IDF TCP library
All functions have comments and are inspired in the Arduino Core. WiFi Library used: [WiFi](https://github.com/urbanze/esp32-wifi)\
Removing some specific lines, this library can work in any system with BSD SOCKET support.

## Simple example to connect in external server
```
WF wifi;
TCP_CLIENT tcp;

wifi.sta_connect("wifi", "1234567890"); //Connect in WiFi

//Connect in port 80 from Google and wait response of GET REQUEST.
if (tcp.connecto("google.com", 80))
{
    ESP_LOGI(__func__, "Connected");
    tcp.printf("GET / HTTP 1.1\r\n\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    char bff[256] = {0};
    tcp.readBytes(bff, sizeof(bff));
    ESP_LOGI(__func__, "Received: %s", bff);
    tcp.stop();
}
else
{
    ESP_LOGE(__func__, "Fail to connect");
}
```

## Simple example to host TCP Server in ESP32
```
WF wifi;
TCP_CLIENT tcp;
TCP_SERVER host;

wifi.sta_connect("wifi", "1234567890"); //Connect in WiFi

host.begin(80); //Host TCP Server in port 80 (common to HTTP WEB SERVERS)
while (1)
{
    //If no client is connected, wait 1sec for a new connection.
    //This will block (this task) until block time end.
    //Higher times is good for a healthy RTOS.
    //Returns immediately when a client connect.
    if (!tcp.connected())
    {
        tcp.get_sock(host.sv(10));
    }

    //When client write data to ESP32, print and echo back.
    if (tcp.available())
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        ESP_LOGI(__func__, "Data available %d", tcp.available());

        char bff[128] = {0};
        tcp.readBytes(bff, sizeof(bff));

        ESP_LOGI(__func__, "Message: %s", bff);
        tcp.printf("ESP32 Received: %s", bff);
    }
}
```
