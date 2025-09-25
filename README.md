如果编译遇到以下报错:
```shell
\MaTouch_ESP32-P4_TFT_with_Touch_7\examples\esp_brookesia_phone\managed_components\espressif__esp_wifi_remote\dummy_src.c.obj.d: No such file or directory
```

启用 Windows 长路径支持。

```cmd
# 以管理员权限运行
reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1
```

重启系统。