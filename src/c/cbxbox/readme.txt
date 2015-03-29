Xbox joypad control of lastest BlueBrain (board without the detachable motor driver and using mbed)



The 2 sub-folders should be copied to /etc/service/

Due to BLE GATT characteristic handle differences between rfduino (0x0011) and mbed (0x0022) the default
gatttool installed by the Raspbeery pi install step needs updating when using the newer boards

The gatttool_v2 should be copied to /usr/local/bin/gatttool


