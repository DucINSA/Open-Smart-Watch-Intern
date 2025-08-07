# OSW Sensor Data Logger

## Description
Application personnalisée pour l'Open-SmartWatch permettant l'acquisition et l'affichage des données des capteurs en temps réel, avec communication serveur distant.

## Fonctionnalités

### Acquisition de données
- **Température** : Lecture du capteur BMA400/BMI270
- **Accélération** : Données X, Y, Z en temps réel
- **Compteur de pas** : Suivi des pas quotidiens
- **Niveau de batterie** : Monitoring de l'état de charge
- **Timestamp** : Horodatage des données

### Communication serveur
- **Envoi HTTP** : Transmission JSON toutes les 5 secondes
- **Gestion d'erreurs** : Reconnexion automatique
- **Statut de connexion** : Affichage en temps réel

### Interface utilisateur
- **Affichage temps réel** : Données mises à jour en continu
- **Mode d'affichage** : Plusieurs vues disponibles
- **Contrôles** : Activation/désactivation auto-update

## Configuration

### Prérequis
- Open-SmartWatch Light Edition V4.0
- Connexion WiFi configurée
- Serveur distant pour recevoir les données

### Installation
1. Compiler avec PlatformIO :
   ```bash
   pio run -e EXPERIMENTAL_LIGHT_EDITION_V4_0 --target upload
   ```

2. Configurer le WiFi dans `config.h` :
   ```cpp
   #define CONFIG_WIFI_SSID "VOTRE_SSID"
   #define CONFIG_WIFI_PASS "VOTRE_MOT_DE_PASSE"
   ```

3. Configurer l'URL du serveur dans l'application

## Utilisation

1. **Lancer l'application** : Naviguer vers "Tools" > "Sensor Logger"
2. **Vérifier la connexion** : Le statut WiFi s'affiche
3. **Observer les données** : Les valeurs se mettent à jour automatiquement
4. **Contrôler l'envoi** : Utiliser les boutons pour activer/désactiver

## Structure des données

### Format JSON envoyé au serveur
```json
{
  "timestamp": 1234567890,
  "temperature": 25.6,
  "acceleration": {
    "x": 0.12,
    "y": -0.05,
    "z": 9.81
  },
  "steps": 1234,
  "battery": 85,
  "device_id": "OSW_1"
}
```

## Développement

### Fichiers principaux
- `include/apps/tools/OswAppSensorDataLogger.h` : Déclaration de la classe
- `src/apps/tools/OswAppSensorDataLogger.cpp` : Implémentation

### Ajout de nouveaux capteurs
1. Modifier la structure `SensorData`
2. Ajouter la lecture dans `updateSensorData()`
3. Mettre à jour `formatSensorData()`

## Contribution

Ce projet suit les [guidelines de contribution Open-SmartWatch](https://open-smartwatch.github.io/howto/contribute/).

### Pour contribuer :
1. Fork du repository principal
2. Créer une branche basée sur `develop`
3. Implémenter les modifications
4. Tester sur matériel réel
5. Soumettre une Pull Request

## Auteur
- **DucINSA** - [GitHub](https://github.com/DucINSA)
- **Projet** : Démonstrateur de Machine Learning et Cybersécurité sur Smartwatch Programmable
- **Date** : 2025

## Licence
Ce projet est sous licence open source, conforme aux standards Open-SmartWatch.
