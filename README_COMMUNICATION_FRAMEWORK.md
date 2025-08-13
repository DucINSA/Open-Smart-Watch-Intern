# Framework de Communication Open-SmartWatch

## Description
Framework flexible pour la communication entre l'Open-SmartWatch et un serveur distant, supportant WiFi et Bluetooth avec gestion automatique des connexions et des files d'attente.

## Architecture

### Composants principaux

#### 1. OswServiceTaskCommunication
Service principal gérant la communication avec les fonctionnalités suivantes :
- **Gestion multi-protocoles** : WiFi et Bluetooth
- **File d'attente intelligente** : Messages mis en file d'attente avec retry automatique
- **Sélection automatique** : Choix automatique du meilleur protocole disponible
- **Heartbeat** : Envoi périodique de signaux de vie
- **Callbacks** : Notifications d'événements (connexion, erreurs, messages reçus)

#### 2. Types de communication
```cpp
enum class CommunicationType {
    WIFI,        // Utilise uniquement WiFi
    BLUETOOTH,   // Utilise uniquement Bluetooth
    AUTO         // Choix automatique selon disponibilité
};
```

#### 3. Types de messages
```cpp
enum class MessageType {
    SENSOR_DATA,    // Données des capteurs
    SYSTEM_STATUS,  // Statut système
    COMMAND,        // Commandes
    RESPONSE,       // Réponses
    ERROR           // Messages d'erreur
};
```

## Utilisation

### 1. Initialisation du service

```cpp
#include "services/OswServiceTaskCommunication.h"

// Récupérer le service
auto commService = OswServiceManager::getInstance().getTask<OswServiceTaskCommunication>();

// Configuration
commService->setCommunicationType(CommunicationType::AUTO);
commService->setServerUrl("http://your-server.com/api");
commService->setDeviceId("OSW_DEVICE_001");

// Callbacks
CommunicationCallbacks callbacks;
callbacks.onMessageReceived = [](const CommunicationMessage& msg) {
    // Traitement des messages reçus
};
callbacks.onConnectionStatusChanged = [](const String& status) {
    // Gestion des changements de statut
};
callbacks.onError = [](const String& error) {
    // Gestion des erreurs
};

commService->setCallbacks(callbacks);
```

### 2. Envoi de données

#### Données des capteurs
```cpp
JsonDocument sensorData;
sensorData["temperature"] = hal->environment()->getTemperature();
sensorData["acceleration_x"] = hal->environment()->getAccelerationX();
sensorData["steps"] = hal->environment()->getStepsToday();
sensorData["battery"] = hal->getBatteryPercent();

commService->sendSensorData(sensorData);
```

#### Statut système
```cpp
JsonDocument systemData;
systemData["uptime"] = millis();
systemData["free_heap"] = ESP.getFreeHeap();
systemData["cpu_freq"] = OswHal::getInstance()->getCPUClock();

commService->sendSystemStatus(systemData);
```

#### Commandes
```cpp
JsonDocument params;
params["action"] = "restart";
params["delay"] = 5000;

commService->sendCommand("system_command", params);
```

### 3. Gestion de la connexion

```cpp
// Connexion manuelle
if (commService->connect()) {
    OSW_LOG_I("Connected successfully");
}

// Vérification du statut
if (commService->isConnected()) {
    String status = commService->getConnectionStatus();
    OSW_LOG_I("Connection status: ", status);
}

// Déconnexion
commService->disconnect();
```

### 4. Configuration avancée

```cpp
// Nombre de tentatives de retry
commService->setRetryCount(5);

// Timeout de la file d'attente (30 secondes)
commService->setQueueTimeout(30000);

// Timeout de connexion (10 secondes)
commService->setConnectionTimeout(10000);
```

## Format des messages

### Structure JSON de base
```json
{
  "message_id": "OSW_DEVICE_001_1234567890_1",
  "device_id": "OSW_DEVICE_001",
  "timestamp": 1234567890,
  "type": 0,
  "sensor_data": {
    "temperature": 25.6,
    "acceleration_x": 0.12,
    "acceleration_y": -0.05,
    "acceleration_z": 9.81,
    "steps": 1234,
    "battery_level": 85,
    "battery_raw": 3200,
    "is_charging": false,
    "ram_used": 45000,
    "ram_total": 327680
  }
}
```

### Headers HTTP (WiFi)
```
Content-Type: application/json
Device-ID: OSW_DEVICE_001
```

## Application de test

### OswAppCommunicationTest
Application intégrée pour tester le framework avec les fonctionnalités suivantes :

#### Écrans disponibles
1. **Status** : État de la connexion et statistiques
2. **Configuration** : Paramètres de communication
3. **Messages** : Statistiques des messages envoyés/reçus
4. **Test** : Fonctions de test manuelles

#### Fonctions de test
- **Bouton 1** : Envoi de données de capteurs
- **Bouton 2** : Envoi de statut système
- **Bouton 3** : Envoi de commande de test

#### Configuration
- **Envoi automatique** : Activation/désactivation
- **Intervalle** : Période entre les envois automatiques
- **Type de communication** : WiFi, Bluetooth ou Auto

## Intégration dans le système

### 1. Enregistrement du service
Le service est automatiquement enregistré dans `OswServiceManager` et disponible pour toutes les applications.

### 2. Enregistrement de l'application de test
```cpp
// Dans main.cpp
main_mainDrawer.registerAppLazy<OswAppCommunicationTest>("Tools");
```

### 3. Configuration des flags de compilation
```ini
# Dans platformio.ini
build_flags =
    -D OSW_FEATURE_WIFI
    -D OSW_FEATURE_BLE_SERVER
```

## Gestion des erreurs

### Types d'erreurs gérées
- **Timeout de connexion** : Reconnexion automatique
- **Échec d'envoi** : Retry avec backoff exponentiel
- **Perte de connexion** : Basculement automatique WiFi ↔ Bluetooth
- **File d'attente pleine** : Suppression des anciens messages

### Logs et monitoring
```cpp
// Logs automatiques
OSW_LOG_I("Message sent successfully: MSG_001");
OSW_LOG_E("Connection error: Timeout");
OSW_LOG_W("Message timeout, removing from queue: MSG_002");
```

## Performance et optimisation

### Gestion de la mémoire
- **File d'attente limitée** : Évite l'accumulation de messages
- **Messages compressés** : JSON optimisé
- **Libération automatique** : Nettoyage des messages expirés

### Gestion de l'énergie
- **Heartbeat adaptatif** : Fréquence ajustée selon l'activité
- **Mode veille** : Désactivation automatique si inactif
- **Optimisation WiFi** : Mode basse consommation si disponible

## Exemples d'utilisation avancée

### 1. Surveillance continue
```cpp
void loop() {
    // Envoi automatique toutes les 30 secondes
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 30000) {
        sendSensorData();
        lastSend = millis();
    }
}
```

### 2. Gestion des événements
```cpp
void onButtonPress() {
    JsonDocument eventData;
    eventData["event"] = "button_press";
    eventData["button"] = "button_1";
    eventData["timestamp"] = millis();
    
    commService->sendSystemStatus(eventData);
}
```

### 3. Synchronisation de données
```cpp
void syncData() {
    // Envoi de toutes les données en attente
    commService->processQueue();
    
    // Vérification du statut
    if (commService->getQueueSize() > 0) {
        OSW_LOG_W("Data sync incomplete, queue size: ", commService->getQueueSize());
    }
}
```

## Développement et extension

### Ajout de nouveaux types de messages
1. Étendre l'enum `MessageType`
2. Ajouter les méthodes d'envoi correspondantes
3. Implémenter le traitement côté serveur

### Support de nouveaux protocoles
1. Créer une nouvelle classe de service
2. Implémenter les méthodes de communication
3. Intégrer dans `OswServiceTaskCommunication`

### Personnalisation des callbacks
```cpp
// Callback personnalisé pour traitement spécifique
callbacks.onMessageReceived = [this](const CommunicationMessage& msg) {
    if (msg.type == MessageType::COMMAND) {
        handleCommand(msg);
    } else if (msg.type == MessageType::SENSOR_DATA) {
        processSensorData(msg);
    }
};
```

## Support et maintenance

### Débogage
- **Logs détaillés** : Activation via `DEBUG=1`
- **Monitoring en temps réel** : Via l'application de test
- **Statistiques** : Compteurs de messages et erreurs

### Mise à jour
- **Configuration dynamique** : Modification sans redémarrage
- **Rollback** : Retour aux paramètres précédents
- **Validation** : Vérification de la cohérence des paramètres

## Licence
Ce framework suit les mêmes conditions de licence que le projet Open-SmartWatch principal.
