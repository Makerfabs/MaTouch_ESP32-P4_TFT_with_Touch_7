#pragma once

#include "esp_log.h"
#include "esp_check.h"
#include "lvgl.h"
#include "file_iterator.h"
#include "esp_brookesia.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Recorder: public ESP_Brookesia_PhoneApp {
    public:
        Recorder();
        ~Recorder();

        bool init(void);
        bool close(void);
        bool back(void);

        bool run(void) override;
        
    private:
        enum RecorderState {
            STATE_IDLE,
            STATE_RECORDING,
            STATE_PAUSED
        };
        
        lv_obj_t *_screen;
        lv_obj_t *_button;
        lv_obj_t *_label_button;
        lv_obj_t *_time_label;
        lv_obj_t *_status_label;

        FILE *_file;
        RecorderState _state;
        int _bytes_recorded;
        int _total_recorded_bytes;
        TaskHandle_t _task_handle;
        volatile bool _stop_task_flag;  // 添加任务停止标志

        static void _button_cb(lv_event_t *e);

        static void app_task(void *arg);
        void _update_display();
        void _write_wav_header();
        void _update_wav_header();
        void _start_recording();
        void _stop_recording();
        const char* _get_time_string(int milliseconds);
};