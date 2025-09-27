# ESP_Brookesia Phone

[中文版本](./README_CN.md)

This example, based on [ESP_Brookesia](https://github.com/espressif/esp-brookesia), demonstrates an Android-like interface that includes many different applications. The example utilizes the development board's MIPI-DSI, MIPI-CSI, ESP32-C6, SD card, and audio interfaces. Based on this example, a use case can be created using ESP_Brookesia, enabling efficient development of multimedia applications.

## Getting Started


### ESP-IDF Required

- This example supports ESP-IDF release/v5.4 and later branches. By default, it runs on ESP-IDF release/v5.4.
- Please follow the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to set up the development environment. **We highly recommend** you [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) to get familiar with ESP-IDF and make sure the environment is set up correctly.

### Configuration

Run ``idf.py menuconfig`` and go to ``Board Support Package(ESP32-P4)``:

```
menuconfig > Component config > Board Support Package
```

To use the SD card and enable the "Recorder" APP, "Camera" APP, run ``idf.py menuconfig`` and then select ``Example Configurations`` > ``Enable SD Card``

## How to Use the Example


### Build and Flash the Example

Build the project and flash it to the board, then run monitor tool to view serial output (replace `PORT` with your board's serial port name):

```c
idf.py -p PORT flash monitor
```

To exit the serial monitor, type ``Ctrl-]``.

See the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.
