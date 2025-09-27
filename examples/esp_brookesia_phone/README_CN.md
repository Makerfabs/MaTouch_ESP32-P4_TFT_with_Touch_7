# ESP_Brookesia Phone

[英文版本](./README.md)

该示例基于 [ESP_Brookesia](https://github.com/espressif/esp-brookesia)，展示了一个类似 Android 的界面，其中包含许多不同的应用程序。该示例使用了开发板的 MIPI-DSI 接口、MIPI-CSI 接口、ESP32-C6、SD 卡和音频接口。基于此示例，可以基于 ESP_Brookesia 创建一个使用案例，从而高效开发多媒体应用程序。

## 快速入门

### ESP-IDF 要求

- 此示例支持 ESP-IDF release/v5.4 及以上版本。默认情况下，在 ESP-IDF release/v5.4 上运行。
- 请参照 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 设置开发环境。**强烈推荐** 通过 [编译第一个工程](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#id8) 来熟悉 ESP-IDF，并确保环境设置正确。

### 配置

运行 ``idf.py menuconfig`` 并修改 ``Board Support Package(ESP32-P4)`` 配置：

```
menuconfig > Component config > Board Support Package
```

若要使用 SD 卡并启用 "Recorder" APP，“Camera” APP，请运行 ``idf.py menuconfig`` 然后选择 ``Example Configurations`` > ``Enable SD Card``

## 如何使用示例


### 编译和烧录示例

编译项目并将其烧录到开发板上，运行监视工具可查看串行端口输出（将 `PORT` 替换为所用开发板的串行端口名）：

```c
idf.py -p PORT flash monitor
```

输入``Ctrl-]`` 可退出串口监视。

有关配置和使用 ESP-IDF 来编译项目的完整步骤，请参阅 [ESP-IDF 快速入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 。
