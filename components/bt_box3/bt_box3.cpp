#include "bt_box3.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace bt_box3 {

static const char *TAG = "bt_box3";

// Nom de l'appareil Bluetooth
#define BT_DEVICE_NAME "ESP32S3-Box3"

// Task pour gérer les événements Bluetooth
#define BT_APP_TASK_STACK_SIZE 3072
#define BT_APP_TASK_PRIORITY 5
#define BT_APP_TASK_NAME "BtAppTask"

// Files d'attente pour les événements
static QueueHandle_t s_bt_app_task_queue = NULL;

// Structure pour les événements Bluetooth
typedef struct {
    uint16_t sig;
    uint16_t event;
    uint32_t param;
} bt_app_msg_t;

// Types d'événements
enum {
    BT_APP_SIG_WORK_DISPATCH = 0,
};

BTBox3Component::BTBox3Component() 
    : bt_initialized(false), 
      bt_task_handle(nullptr),
      audio_callback(nullptr),
      audio_callback_arg(nullptr) {
    memset(&audio_buffer, 0, sizeof(audio_buffer_t));
}

BTBox3Component::~BTBox3Component() {
    if (this->bt_task_handle) {
        vTaskDelete(this->bt_task_handle);
    }
    
    if (audio_buffer.buffer) {
        free(audio_buffer.buffer);
    }
    
    if (audio_buffer.buffer_semaphore) {
        vSemaphoreDelete(audio_buffer.buffer_semaphore);
    }
    
    if (audio_buffer.data_queue) {
        vQueueDelete(audio_buffer.data_queue);
    }
    
    if (bt_initialized) {
        esp_a2d_sink_deinit();
        esp_avrc_ct_deinit();
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
    }
}

void BTBox3Component::setup() {
    ESP_LOGI(TAG, "Setting up Bluetooth Audio for ESP32-S3 Box 3...");
    
    // Initialiser le buffer audio
    this->audio_buffer.buffer_size = SAMPLE_BUFFER_SIZE * 4; // 16-bit stéréo (2 octets * 2 canaux)
    this->audio_buffer.buffer = (uint8_t*)heap_caps_malloc(this->audio_buffer.buffer_size, MALLOC_CAP_DMA);
    if (!this->audio_buffer.buffer) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        return;
    }
    
    this->audio_buffer.buffer_semaphore = xSemaphoreCreateBinary();
    this->audio_buffer.data_queue = xQueueCreate(4, sizeof(size_t));
    
    // Donner le sémaphore initialement
    xSemaphoreGive(this->audio_buffer.buffer_semaphore);
    
    // Créer la file d'attente pour les événements Bluetooth
    s_bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    if (s_bt_app_task_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Bluetooth event queue");
        return;
    }
    
    // Créer la tâche pour gérer les événements Bluetooth
    xTaskCreate(bt_app_task_handler, BT_APP_TASK_NAME, BT_APP_TASK_STACK_SIZE, this,
                BT_APP_TASK_PRIORITY, &this->bt_task_handle);
    
    // Initialiser le Bluetooth
    init_bluetooth();
    
    ESP_LOGI(TAG, "Bluetooth Audio setup complete");
}

void BTBox3Component::register_audio_callback(void (*callback)(void*, void*, size_t), void* arg) {
    this->audio_callback = callback;
    this->audio_callback_arg = arg;
    ESP_LOGI(TAG, "Audio callback registered");
}

void BTBox3Component::init_bluetooth() {
    ESP_LOGI(TAG, "Initializing Bluetooth...");
    
    // Initialiser le contrôleur Bluetooth
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_bt_controller_init(&bt_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluetooth controller: %s", esp_err_to_name(err));
        return;
    }
    
    err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluetooth controller: %s", esp_err_to_name(err));
        return;
    }
    
    // Initialiser le Bluedroid
    err = esp_bluedroid_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(err));
        return;
    }
    
    err = esp_bluedroid_enable();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(err));
        return;
    }
    
    // Définir le nom de l'appareil Bluetooth
    esp_bt_dev_set_device_name(BT_DEVICE_NAME);
    
    // Enregistrer les callbacks pour les événements Bluetooth
    esp_bt_gap_register_callback(NULL);
    
    // Initialiser A2DP Sink
    err = esp_a2d_sink_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize A2DP sink: %s", esp_err_to_name(err));
        return;
    }
    
    // Enregistrer le callback pour A2DP
    esp_a2d_register_callback(bt_app_a2d_cb);
    esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);
    
    // Initialiser AVRCP Controller
    err = esp_avrc_ct_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize AVRCP controller: %s", esp_err_to_name(err));
        return;
    }
    
    // Enregistrer le callback pour AVRCP
    esp_avrc_ct_register_callback(bt_app_rc_ct_cb);
    
    // Définir la visibilité Bluetooth
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    
    ESP_LOGI(TAG, "Bluetooth initialized successfully");
    this->bt_initialized = true;
}

void BTBox3Component::loop() {
    // Dans loop(), nous pouvons vérifier l'état de la connexion Bluetooth
    // et traiter les données audio si nécessaire
    
    // Vérifier s'il y a des données en attente dans la queue
    size_t data_size;
    if (xQueueReceive(this->audio_buffer.data_queue, &data_size, 0) == pdTRUE) {
        // Traiter les données si nécessaire
        // Cette partie peut être adaptée en fonction de vos besoins
    }
}

void BTBox3Component::dump_config() {
    ESP_LOGCONFIG(TAG, "Bluetooth Audio (ESP32-S3 Box 3) Config:");
    ESP_LOGCONFIG(TAG, "Bluetooth Initialized: %s", this->bt_initialized ? "Yes" : "No");
    ESP_LOGCONFIG(TAG, "Device Name: %s", BT_DEVICE_NAME);
}

// Handler statique pour la tâche Bluetooth
void BTBox3Component::bt_app_task_handler(void *arg) {
    BTBox3Component *self = static_cast<BTBox3Component*>(arg);
    bt_app_msg_t msg;
    
    for (;;) {
        if (xQueueReceive(s_bt_app_task_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.sig == BT_APP_SIG_WORK_DISPATCH) {
                switch (msg.event) {
                    case BT_APP_WORK_EVT_A2DP:
                        // Traiter l'événement A2DP
                        break;
                    case BT_APP_WORK_EVT_AVRCP:
                        // Traiter l'événement AVRCP
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

// Callback pour les événements A2DP
void BTBox3Component::bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    // Cette fonction est statique, mais nous avons besoin de l'instance
    // Nous pouvons utiliser un objet global ou un autre mécanisme pour accéder à l'instance
    
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT:
            ESP_LOGI(TAG, "A2DP connection state: %d", param->conn_stat.state);
            if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
                ESP_LOGI(TAG, "A2DP connected");
            } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
                ESP_LOGI(TAG, "A2DP disconnected");
            }
            break;
            
        case ESP_A2D_AUDIO_STATE_EVT:
            ESP_LOGI(TAG, "A2DP audio state: %d", param->audio_stat.state);
            break;
            
        case ESP_A2D_AUDIO_CFG_EVT:
            ESP_LOGI(TAG, "A2DP audio config: %d Hz, %d bit, %d channel",
                   param->audio_cfg.mcc.cie.sbc[0] & 0x0F,
                   param->audio_cfg.mcc.cie.sbc[1] & 0x03,
                   param->audio_cfg.mcc.cie.sbc[1] >> 2 & 0x03);
            break;
            
        default:
            ESP_LOGI(TAG, "A2DP event: %d", event);
            break;
    }
}

// Callback pour les données A2DP
void BTBox3Component::bt_app_a2d_data_cb(const uint8_t *data, uint32_t len) {
    // Cette fonction est statique, mais nous avons besoin de l'instance
    // Nous pouvons utiliser un objet global ou un autre mécanisme pour accéder à l'instance
    
    // Traiter les données audio reçues via Bluetooth
    // Ces données devraient être envoyées à l'ES8311 DAC
    
    ESP_LOGD(TAG, "A2DP data received: %d bytes", len);
    
    // Ici, vous devriez implémenter le code pour envoyer les données au DAC
    // Par exemple, utiliser le callback audio enregistré
}

// Callback pour les événements AVRCP
void BTBox3Component::bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param) {
    // Cette fonction est statique, mais nous avons besoin de l'instance
    // Nous pouvons utiliser un objet global ou un autre mécanisme pour accéder à l'instance
    
    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT:
            ESP_LOGI(TAG, "AVRCP connection state: %d", param->conn_stat.connected);
            break;
            
        case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
            ESP_LOGI(TAG, "AVRCP passthrough response: %d", param->psth_rsp.key_code);
            break;
            
        case ESP_AVRC_CT_METADATA_RSP_EVT:
            ESP_LOGI(TAG, "AVRCP metadata response");
            break;
            
        case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
            ESP_LOGI(TAG, "AVRCP notification");
            break;
            
        default:
            ESP_LOGI(TAG, "AVRCP event: %d", event);
            break;
    }
}

void BTBox3Component::send_audio_to_dac(uint8_t *data, size_t size) {
    // Vérifier si nous pouvons prendre le sémaphore du buffer
    if (xSemaphoreTake(this->audio_buffer.buffer_semaphore, 0) != pdTRUE) {
        // Le buffer est occupé, ignorer ces données
        return;
    }
    
    // S'assurer que la taille ne dépasse pas la capacité du buffer
    size_t copy_size = (size > this->audio_buffer.buffer_size) ? this->audio_buffer.buffer_size : size;
    memcpy(this->audio_buffer.buffer, data, copy_size);
    
    // Si nous avons un callback audio enregistré, l'appeler
    if (this->audio_callback) {
        this->audio_callback(this->audio_callback_arg, this->audio_buffer.buffer, copy_size);
    }
    
    // Ajouter les données à la queue pour d'autres traitements si nécessaire
    xQueueSend(this->audio_buffer.data_queue, &copy_size, portMAX_DELAY);
    
    // Relâcher le sémaphore du buffer
    xSemaphoreGive(this->audio_buffer.buffer_semaphore);
    
    ESP_LOGD(TAG, "Audio data sent to DAC (size: %d bytes)", copy_size);
}

void BTBox3Component::handle_stack_event(uint16_t event, void *p_param) {
    // Gérer les événements de la pile Bluetooth
}

void BTBox3Component::handle_a2d_event(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    // Gérer les événements A2DP
}

void BTBox3Component::handle_a2d_data(const uint8_t *data, uint32_t len) {
    // Gérer les données audio A2DP
    send_audio_to_dac((uint8_t*)data, len);
}

void BTBox3Component::handle_rc_ct_event(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param) {
    // Gérer les événements AVRCP
}

}  // namespace bt_box3
}  // namespace esphome
