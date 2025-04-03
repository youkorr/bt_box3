# ESP32-S3 Box 3 Bluetooth Audio Receiver

Ce projet permet d'utiliser l'ESP32-S3 Box 3 comme un récepteur audio Bluetooth, envoyant le son reçu vers le DAC ES8311 intégré.

## Problème résolu

Le code original utilisant l'USB Host pour connecter un adaptateur USB-A vers prise casque 3,5 mm présentait des problèmes de démarrage, probablement en raison d'un conflit avec l'interface USB_SERIAL_JTAG utilisée par ESPHome pour le logging.

Cette nouvelle implémentation abandonne l'approche USB Host au profit d'une connexion Bluetooth, permettant de connecter directement des appareils Bluetooth comme des smartphones ou ordinateurs sans adaptateur physique.

## Structure des fichiers

- `bt_box3.h` - Fichier d'en-tête du composant Bluetooth Audio
- `bt_box3.cpp` - Implémentation du composant Bluetooth Audio
- `esp32s3box3.yaml` - Configuration ESPHome pour l'ESP32-S3 Box 3

## Installation

1. Créez un dossier `components/bt_box3` dans votre projet ESPHome
2. Placez-y les fichiers `bt_box3.h` et `bt_box3.cpp`
3. Utilisez le fichier YAML fourni comme base pour votre configuration
4. Compilez et téléversez vers votre ESP32-S3 Box 3

## Comment ça fonctionne

1. Le composant initialise le Bluetooth et se met en mode découvrable
2. L'appareil apparaît sous le nom "ESP32S3-Box3" dans la liste des appareils Bluetooth
3. Connectez-vous depuis votre smartphone/ordinateur
4. Le son est automatiquement transmis à l'ES8311 DAC et joué sur le haut-parleur

## Configuration requise

- ESP-IDF 5.1.5
- ESPHome (dernière version)
- Un ESP32-S3 Box 3

## Modifications clés

Par rapport à l'implémentation USB Host originale :

1. Utilisation de l'API Bluetooth d'ESP-IDF au lieu de l'API USB Host
2. Support du profil A2DP Sink pour recevoir l'audio
3. Support du profil AVRCP pour le contrôle de la lecture
4. Désactivation de l'USB CDC dans sdkconfig pour éviter les conflits
5. Flux de données audio simple de Bluetooth vers le DAC ES8311

## Dépannage

Si vous rencontrez des problèmes :

- Vérifiez que le Bluetooth est activé dans votre configuration ESP-IDF
- Assurez-vous que les ports GPIO sont correctement configurés pour l'ES8311
- Vérifiez les logs pour voir si le Bluetooth s'initialise correctement
- Essayez de redémarrer l'ESP32-S3 Box 3 si la connexion échoue

## Améliorations futures

- Ajouter le support pour les métadonnées des pistes (titre, artiste, etc.)
- Améliorer la gestion des boutons physiques pour contrôler la lecture
- Implémenter la gestion de plusieurs appareils connectés
- Optimiser la qualité audio et réduire la latence
