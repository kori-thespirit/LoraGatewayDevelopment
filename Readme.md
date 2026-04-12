# Wi-Fi Unit Test Module - Lora Gateway Development

This component is designed to be integrated into the **ESP-IDF v5.x** framework for testing and validating Wi-Fi connectivity on ESP32-based hardware. It is part of the **SCRUM-37** task for the Lora Gateway project.

## What is it
The **Wi-Fi Unit Test Module** is a C-based implementation for ESP32 that handles network scanning, station connection (STA mode), and event-driven communication. It ensures the Wi-Fi hardware is functional before being integrated with LoRa, Ethernet, and LTE layers.

## How to install
Simply clone the repository and navigate to the specific branch:

```bash
git clone  https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment

```
## Configuration
Using 'idf.py menuconfig', you can configure the Wi-Fi credentials under the "Example Connection Configuration" section:  
SSID: Your Wi-Fi name.   
Password: Your Wi-Fi password.    
Max Retries: Number of reconnection attempts.   