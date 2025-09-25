#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_check.h"
#include "lvgl.h"
#include "esp_brookesia.hpp"

#include "usb/usb_host.h"
#include "esp_private/usb_phy.h"
#include "errno.h"

#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"

typedef enum {
    APP_EVENT = 0,
    APP_EVENT_HID_HOST
} app_event_group_t;

typedef struct {
    app_event_group_t event_group;
    /* HID Host - Device related info */
    struct {
        hid_host_device_handle_t handle;
        hid_host_driver_event_t event;
        void *arg;
    } hid_host_device;
} app_event_queue_t;

typedef enum {
    KEY_STATE_PRESSED = 0x00,
    KEY_STATE_RELEASED = 0x01
} key_state_t;

/**
 * @brief Key event
 */
typedef struct {
    key_state_t state;
    uint8_t modifier;
    uint8_t key_code;
} key_event_t;

// 键盘布局结构
typedef struct {
    uint8_t key_code;
    const char* label;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} key_layout_t;

class UsbTest: public ESP_Brookesia_PhoneApp {
    public:
        UsbTest();
        ~UsbTest();

        bool init(void);
        bool close(void);
        bool back(void);

        bool run(void) override;

        volatile bool _is_closing;

    private:
        QueueHandle_t _app_event_queue;

        // LVGL界面对象
        lv_obj_t* _main_screen;
        lv_obj_t* _title_label;
        lv_obj_t* _mouse_area;
        lv_obj_t* _mouse_label;
        lv_obj_t* _device_info_label;
        
        // 键盘相关
        lv_obj_t* _keyboard_area;
        lv_obj_t* _key_buttons[90];  // 86键 + 一些额外的
        int _total_keys;
        
        // 键盘布局定义
        static const key_layout_t keyboard_layout[];
        static const int keyboard_layout_size;

        void hid_host_device_event(hid_host_device_handle_t hid_device_handle,
                           const hid_host_driver_event_t event,
                           void *arg);

        static void hid_host_device_callback(hid_host_device_handle_t hid_device_handle,
                              const hid_host_driver_event_t event,
                              void *arg);
        static void usb_lib_task(void *arg);
        static void event_task(void *arg);
        static void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle,
                                 const hid_host_interface_event_t event,
                                 void *arg);
        static void hid_host_keyboard_report_callback(const uint8_t *const data, const int length);
        static void key_event_callback(key_event_t *key_event);
        static void hid_print_new_device_report_header(hid_protocol_t proto);
        static void hid_host_mouse_report_callback(const uint8_t *const data, const int length);
        static void hid_host_generic_report_callback(const uint8_t *const data, const int length);

        // UI更新方法
        void update_mouse_ui(int x, int y, bool button1, bool button2);
        void update_device_info(const char* device_info);
        void create_keyboard_layout();
        void highlight_key(uint8_t key_code, bool pressed);
        int find_key_index(uint8_t key_code);
};