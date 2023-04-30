# Smart Battery Monitor
Arduino &amp; CANBus Shield Based Smart Battery Monitor for Tattu 14S LiPo Battery.

This project uses Arduino platform and [CANBus Shield](https://github.com/Seeed-Studio/Seeed_Arduino_CAN) to connect and monitor Tattu 14S LiPo Smart Battery through CAN Bus. It reports battery data to Serial Port and 128x64 OLED Screen.

The OLED Screen uses [GyverOLED](https://github.com/GyverLibs/GyverOLED) library and requires more than 2k RAM memory therefore it is available only on MEGA 2560 boards.

Tattu Smart Battery reports data via CAN Bus using [Cyphal](https://opencyphal.org/) (aka DroneCAN or UAVCAN) Protocol. It reports the following attributes:

1. Manufacturer ID
2. SKU Code
3. Battery Voltage (mV)
4. Battery Current (10mA, +: Charging, -: Discharging)
5. Pack Temperature (1C)
6. Remaining Capacity (%)
7. Cycle Count (times)
8. Health Status (%)
9. Cell Voltages (1-14)
10. Remaining Capacity (mA)
11. Standard Capacity (mA)
12. Error Information

Error Information consists of 12 bit data:

* Bit 0: Low Temperature
* Bit 1: Over Temperature
* Bit 2: Over Current while Charging
* Bit 3: Over Current while Discharging
* Bit 4: Total Voltage is too Low
* Bit 5: Total Voltage is too High
* Bit 6: Voltage Difference of cells too High
* Bit 7: Single Cell voltage is too High
* Bit 8: Single Cell voltage is too Low
* Bit 9: Short Circuit while Charging
* Bit 10: Short Circuit while Discharging
* Bit 11: Using non-original Charger

![pic4](https://user-images.githubusercontent.com/2091144/234117830-420b475c-1d76-4f0a-aeb5-f673f146d88d.jpg)
![pic5](https://user-images.githubusercontent.com/2091144/234117850-acc75a34-2dc7-43a4-ace2-64c084d38196.jpg)


![pic1](https://user-images.githubusercontent.com/2091144/234117009-285e143e-8ea4-49b8-9a33-e0eb13bc2d32.jpg)
![pic2](https://user-images.githubusercontent.com/2091144/234117017-18d0b96f-ad74-4548-b4a7-4bd166df5eb8.jpg)

You may see example Serial Output in the next image:

![pic3](https://user-images.githubusercontent.com/2091144/234116938-acbc139d-b9d6-4a0f-95a4-0e8717769cb6.jpg)
