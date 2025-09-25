#include "Recorder.hpp"

#include "bsp_board_extra.h"

#include "format_wav.h"

// Declare external image resource
LV_IMG_DECLARE(img_app_recorder);

static const char *TAG = "Recorder";

// WAV文件参数 (采样率16kHz, 位深度16, 单声道)
#define SAMPLE_RATE 16000
#define BIT_DEPTH 16
#define CHANNELS 1
#define BYTE_RATE (SAMPLE_RATE * CHANNELS * BIT_DEPTH / 8)

Recorder::Recorder():
    ESP_Brookesia_PhoneApp("Recorder", &img_app_recorder, true),
    _screen(nullptr),
    _button(nullptr),
    _label_button(nullptr),
    _time_label(nullptr),
    _status_label(nullptr),
    _file(nullptr),
    _state(STATE_IDLE),
    _bytes_recorded(0),
    _total_recorded_bytes(0),
    _task_handle(nullptr),
    _stop_task_flag(false)
{

}

Recorder::~Recorder()
{
    if (_state == STATE_RECORDING || _state == STATE_PAUSED) {
        _stop_recording();
    }
    
    if (_task_handle != nullptr) {
        _stop_task_flag = true;

        vTaskDelay(pdMS_TO_TICKS(100));
        if (_task_handle != nullptr) {
            vTaskDelete(_task_handle);
            _task_handle = nullptr;
        }
    }
    
    if (_file != nullptr) {
        fclose(_file);
        _file = nullptr;
    }
}

bool Recorder::run(void)
{
    bsp_extra_codec_handle_stop(1);
    _screen = lv_scr_act();
    lv_obj_clean(_screen);

    // Set background color
    lv_obj_set_style_bg_color(_screen, lv_color_hex(0x1a1a2e), LV_PART_MAIN);

    // Create title
    lv_obj_t *title = lv_label_create(_screen);
    lv_label_set_text(title, "Voice Recorder");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Create time display label
    _time_label = lv_label_create(_screen);
    lv_label_set_text(_time_label, "00:00:00");
    lv_obj_set_style_text_color(_time_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(_time_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_align(_time_label, LV_ALIGN_TOP_MID, 0, 70);

    // Create status label
    _status_label = lv_label_create(_screen);
    lv_label_set_text(_status_label, "Ready to record");
    lv_obj_set_style_text_color(_status_label, lv_color_hex(0xcccccc), LV_PART_MAIN);
    lv_obj_align(_status_label, LV_ALIGN_TOP_MID, 0, 120);

    // Create main record button
    _button = lv_btn_create(_screen);
    lv_obj_set_size(_button, 120, 120);
    lv_obj_set_style_radius(_button, 60, LV_PART_MAIN);
    lv_obj_align(_button, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(_button, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(_button, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(_button, lv_color_hex(0xc0392b), LV_PART_MAIN);
    lv_obj_add_event_cb(_button, _button_cb, LV_EVENT_CLICKED, this);

    _label_button = lv_label_create(_button);
    lv_label_set_text(_label_button, "Start");
    lv_obj_set_style_text_color(_label_button, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(_label_button, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(_label_button);

    return true;
}

bool Recorder::init(void)
{
    esp_err_t ret = bsp_extra_codec_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize codec: %s", esp_err_to_name(ret));
        return false;
    }
    
    _state = STATE_IDLE;
    _bytes_recorded = 0;
    _total_recorded_bytes = 0;
    _stop_task_flag = false;
    return true; 
}

bool Recorder::back(void)
{
    if (_state == STATE_RECORDING || _state == STATE_PAUSED) {
        _stop_recording();
    }
    notifyCoreClosed();
    return true;
}

bool Recorder::close(void)
{
    if (_state == STATE_RECORDING || _state == STATE_PAUSED) {
        _stop_recording();
    }
    
    esp_err_t ret = bsp_extra_codec_dev_stop();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to stop codec device: %s", esp_err_to_name(ret));
    }
    
    return true;
}

void Recorder::_button_cb(lv_event_t *e)
{
    Recorder *instance = static_cast<Recorder *>(lv_event_get_user_data(e));
    if (instance == nullptr) return;

    switch (instance->_state) {
        case STATE_IDLE:
            instance->_start_recording();
            break;
        case STATE_RECORDING:
        case STATE_PAUSED:
            instance->_stop_recording();
            break;
    }
}

void Recorder::_start_recording()
{
    if (_state != STATE_IDLE) return;

    ESP_LOGI(TAG, "Starting recording...");
    
    _file = fopen("/sdcard/music/record.wav", "wb");
    if (_file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for recording");
        lv_label_set_text(_status_label, "File open failed");
        return;
    }

    _write_wav_header();
    _bytes_recorded = 0;
    _total_recorded_bytes = 0;
    _stop_task_flag = false;

    bsp_extra_codec_mute_set(false);
    esp_err_t ret = bsp_extra_codec_set_fs(SAMPLE_RATE, BIT_DEPTH, I2S_SLOT_MODE_MONO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set codec sample rate: %s", esp_err_to_name(ret));
        fclose(_file);
        _file = nullptr;
        lv_label_set_text(_status_label, "Codec config failed");
        return;
    }
    
    _state = STATE_RECORDING;
    _update_display();
    
    BaseType_t result = xTaskCreate(app_task, "app_task", 4096, this, 5, &_task_handle);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create recording task");
        _state = STATE_IDLE;
        fclose(_file);
        _file = nullptr;
        _update_display();
    } else {
        ESP_LOGI(TAG, "Recording task created successfully");
    }
}

void Recorder::_stop_recording()
{
    if (_state == STATE_IDLE) return;
    
    ESP_LOGI(TAG, "Stopping recording...");
    
    _stop_task_flag = true;

    if (_task_handle != nullptr) {
        for (int i = 0; i < 50 && _task_handle != nullptr; i++) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        if (_task_handle != nullptr) {
            vTaskDelete(_task_handle);
            _task_handle = nullptr;
        }
    }
    
    // Set state to IDLE
    _state = STATE_IDLE;
    
    if (_file != nullptr) {
        if (_total_recorded_bytes > 0) {
            _update_wav_header();
            ESP_LOGI(TAG, "Recorded total bytes: %d", _total_recorded_bytes);
        }
        fclose(_file);
        _file = nullptr;
    }
    
    esp_err_t ret = bsp_extra_codec_dev_stop();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to stop codec device: %s", esp_err_to_name(ret));
    }

    _update_display();
    
    ESP_LOGI(TAG, "Recording stopped successfully");
}

void Recorder::_update_display()
{
    if (_screen == nullptr) return;
    
    switch (_state) {
        case STATE_IDLE:
            if (_label_button) lv_label_set_text(_label_button, "Start");
            if (_button) lv_obj_set_style_bg_color(_button, lv_color_hex(0xe74c3c), LV_PART_MAIN);
            if (_status_label) lv_label_set_text(_status_label, "Ready to record");
            if (_time_label) lv_label_set_text(_time_label, "00:00:00");
            break;
            
        case STATE_RECORDING:
            if (_label_button) lv_label_set_text(_label_button, "Stop");
            if (_button) lv_obj_set_style_bg_color(_button, lv_color_hex(0x2ecc71), LV_PART_MAIN);
            if (_status_label) lv_label_set_text(_status_label, "Recording...");
            break;
            
        case STATE_PAUSED:
            if (_label_button) lv_label_set_text(_label_button, "Stop");
            if (_button) lv_obj_set_style_bg_color(_button, lv_color_hex(0x2ecc71), LV_PART_MAIN);
            if (_status_label) lv_label_set_text(_status_label, "Paused");
            break;
    }
}

void Recorder::_write_wav_header()
{
    if (_file == nullptr) {
        return;
    }
    
    wav_header_t wav_header = WAV_HEADER_PCM_DEFAULT(0, BIT_DEPTH, SAMPLE_RATE, CHANNELS);
    
    // 写入WAV头部
    size_t written = fwrite(&wav_header, sizeof(wav_header_t), 1, _file);
    if (written != 1) {
        ESP_LOGE(TAG, "Failed to write WAV header");
    }
    fflush(_file);
}

void Recorder::_update_wav_header()
{
    if (_file == nullptr || _total_recorded_bytes == 0) {
        return;
    }
    
    // 保存当前文件位置
    long current_pos = ftell(_file);
    
    // 回到文件开头
    fseek(_file, 0, SEEK_SET);
    
    wav_header_t wav_header = WAV_HEADER_PCM_DEFAULT(0, BIT_DEPTH, SAMPLE_RATE, CHANNELS);

    // 计算文件大小: 总字节数 + 44 - 8 (RIFF头部不包括前8个字节)
    wav_header.descriptor_chunk.chunk_size = _total_recorded_bytes + sizeof(wav_header_t) - 8;
    wav_header.data_chunk.subchunk_size = _total_recorded_bytes;

    // 写入WAV头部
    size_t written = fwrite(&wav_header, sizeof(wav_header_t), 1, _file);
    if (written != 1) {
        ESP_LOGE(TAG, "Failed to update WAV header");
    }
    
    // 恢复文件位置
    fseek(_file, current_pos, SEEK_SET);
    
    fflush(_file);
    
    ESP_LOGI(TAG, "WAV header updated - Total size: %d bytes", _total_recorded_bytes);
}

const char* Recorder::_get_time_string(int milliseconds)
{
    static char time_str[12];
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 100;
    
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
    return time_str;
}

void Recorder::app_task(void *data)
{
    Recorder *instance = static_cast<Recorder *>(data);
    ESP_LOGI(TAG, "Recording task started");
    
    // 尝试从PSRAM分配缓冲区
    uint8_t *buffer = (uint8_t *)heap_caps_malloc(2048, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    if (buffer == NULL) {
        // 如果无法从PSRAM分配，则尝试从内部内存分配
        buffer = (uint8_t *)malloc(2048);
    }
    
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        if (instance->_file != nullptr) {
            fclose(instance->_file);
            instance->_file = nullptr;
        }
        instance->_state = STATE_IDLE;
        instance->_update_display();
        instance->_task_handle = nullptr;
        vTaskDelete(NULL);
        return;
    }

    TickType_t last_time_update = xTaskGetTickCount();
    int elapsed_time = 0;

    while (!instance->_stop_task_flag && instance->_state != STATE_IDLE) {
        if (instance->_state == STATE_PAUSED) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        size_t bytes_read = 0;
        esp_err_t result = bsp_extra_i2s_read(buffer, 2048, &bytes_read, 1000);
        
        if (result != ESP_OK) {
            if (result == ESP_ERR_TIMEOUT) {
                ESP_LOGD(TAG, "I2S read timeout");
            } else {
                ESP_LOGW(TAG, "I2S read error: %s", esp_err_to_name(result));
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (bytes_read > 0 && instance->_file != nullptr) {
            size_t bytes_written = fwrite(buffer, 1, bytes_read, instance->_file);
            if (bytes_written != bytes_read) {
                ESP_LOGE(TAG, "File write error: wrote %d of %d bytes", bytes_written, bytes_read);
            } else {
                instance->_bytes_recorded += bytes_read;
                instance->_total_recorded_bytes += bytes_read;
            }
            
            // 定期刷新以确保数据写入
            if (instance->_total_recorded_bytes % 16384 == 0) {
                fflush(instance->_file);
            }
        }

        // 每秒更新时间显示
        TickType_t current_time = xTaskGetTickCount();
        if (current_time - last_time_update >= pdMS_TO_TICKS(1000)) {
            elapsed_time += 1000;
            if (instance->_time_label != nullptr) {
                lv_label_set_text(instance->_time_label, instance->_get_time_string(elapsed_time));
            }
            last_time_update = current_time;
            
            ESP_LOGI(TAG, "Recorded %d bytes, elapsed: %s", 
                    instance->_total_recorded_bytes, 
                    instance->_get_time_string(elapsed_time));
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }

    free(buffer);
    
    ESP_LOGI(TAG, "Recording task finished, total bytes: %d", instance->_total_recorded_bytes);
    
    // 清空任务句柄
    instance->_task_handle = nullptr;
    
    vTaskDelete(NULL);
}