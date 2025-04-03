esphome:
  name: esp32s3box3
  friendly_name: ESP32-S3 Box 3

esp32:
  board: esp32-s3-box-3
  framework:
    type: esp-idf
    version: 5.1.5
    sdkconfig_options:
      # Désactiver l'USB CDC pour éviter les conflits avec le Bluetooth
      CONFIG_ESP_CONSOLE_USB_CDC: n
      # Activer le Bluetooth
      CONFIG_BT_ENABLED: y
      CONFIG_BT_CLASSIC_ENABLED: y
      CONFIG_BT_A2DP_ENABLE: y
      CONFIG_BT_A2DP_SINK_ENABLE: y
      CONFIG_BT_BLUEDROID_ENABLED: y
      # Augmenter la taille de la pile MAIN
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: 4096

# Logger et autres composants de base
logger:
  level: INFO
  hardware_uart: UART0

api:
  encryption:
    key: "votre_clé_de_chiffrement"

ota:
  password: "votre_mot_de_passe_ota"

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  # Fallback en mode AP si pas de connexion
  ap:
    ssid: "ESP32S3Box3 Fallback"
    password: "votre_mot_de_passe_ap"

# Configuration pour le DAC ES8311
i2c:
  sda: GPIO8
  scl: GPIO18
  scan: true

# Media Player avec ES8311
audio:
  - platform: es8311
    name: "ESP32-S3 Box 3 Speaker"
    i2s_dout_pin: GPIO17
    i2s_bclk_pin: GPIO16
    i2s_lrclk_pin: GPIO47
    mclk_pin: GPIO40

# Composant personnalisé pour le Bluetooth Audio
external_components:
  - source: 
      type: local
      path: components
    components: [ bt_box3 ]

# Ajout du composant personnalisé
bt_box3:

# Boutons pour contrôler la lecture BT
binary_sensor:
  - platform: gpio
    pin: GPIO0
    name: "Play/Pause"
    on_press:
      then:
        - logger.log: "Play/Pause Button Pressed"
        # Ajouter ici la commande pour envoyer Play/Pause via AVRCP
  
  - platform: gpio
    pin: GPIO13
    name: "Volume Up"
    on_press:
      then:
        - logger.log: "Volume Up Button Pressed"
        # Ajouter ici la commande pour augmenter le volume
  
  - platform: gpio
    pin: GPIO14
    name: "Volume Down"
    on_press:
      then:
        - logger.log: "Volume Down Button Pressed"
        # Ajouter ici la commande pour diminuer le volume

# Capteurs pour l'état de la connexion Bluetooth
sensor:
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 60s

# Indicateurs d'état
text_sensor:
  - platform: template
    name: "Bluetooth Status"
    id: bluetooth_status
    icon: mdi:bluetooth
