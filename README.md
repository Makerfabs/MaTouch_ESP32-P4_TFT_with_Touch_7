If you encounter the following error when compiling under Windows, please try enabling Windows long path support.
```txt
\MaTouch_ESP32-P4_TFT_with_Touch_7\examples\esp_brookesia_phone\managed_components\espressif__esp_wifi_remote\dummy_src.c.obj.d: No such file or directory
```

do :

```cmd
# Open PowerShell with administrator privileges
reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1
```

Restart the system.