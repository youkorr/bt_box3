#pragma once

#include "esphome/core/component.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

namespace esphome {
namespace bt_box3 {

#define SAMPLE_BUFFER_SIZE 2048

typedef struct {
    uint8_t *buffer;
    size_t buffer_size;
    SemaphoreHandle_t buffer_semaphore;
    QueueHandle_t data_queue;
} audio_buffer_t;

class BTBox3Component : public Component {
public:
    BTBox3Component();
    ~BTBox3Component();

    void setup() override;
    void loop() override;
    void dump_config() override;
    
    // Méthode pour enregistrer un callback audio
    void register_audio_callback(void (*callback)(void*, void*, size_t), void* arg);

protected:
    // Méthodes pour l'initialisation Bluetooth
    void init_bluetooth();
    
    // Traitement audio
    void send_audio_to_dac(uint8_t *data, size_t size);
    
    // Callbacks statiques pour Bluetooth A2DP et AVRCP
    static void bt_app_task_handler(void *arg);
    static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);
    static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
    static void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len);
    static void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
    
    // Implémentations des callbacks
    void handle_stack_event(uint16_t event, void *p_param);
    void handle_a2d_event(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
    void handle_a2d_data(const uint8_t *data, uint32_t len);
    void handle_rc_ct_event(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);

private:
    TaskHandle_t bt_task_handle;
    bool bt_initialized;
    
    // Buffer audio et gestion
    audio_buffer_t audio_buffer;
    
    // Callback audio
    void (*audio_callback)(void*, void*, size_t);
    void* audio_callback_arg;
};

}  // namespace bt_box3
}  // namespace esphome
