# Serveur de Test pour le Framework de Communication

## Description
Serveur Node.js simple pour tester le framework de communication Open-SmartWatch. Ce serveur reçoit les données des capteurs et permet de visualiser les communications en temps réel.

## Installation

### Prérequis
- Node.js (version 14 ou supérieure)
- npm ou yarn

### Installation des dépendances
```bash
cd examples/server
npm install
```

## Utilisation

### Démarrage du serveur
```bash
# Mode production
npm start

# Mode développement (avec rechargement automatique)
npm run dev
```

Le serveur démarre sur `http://localhost:8080`

### Endpoints disponibles

#### 1. Statut du serveur
```
GET /
```
Retourne le statut général du serveur et les statistiques.

#### 2. Réception des données
```
POST /api/data
Headers: Device-ID: OSW_DEVICE_001
Body: JSON avec les données des capteurs
```

#### 3. Récupération des données
```
GET /api/data?limit=50&device_id=OSW_DEVICE_001
```

#### 4. Statut des devices
```
GET /api/devices
```

#### 5. Envoi de commandes
```
POST /api/command
Body: {
  "device_id": "OSW_DEVICE_001",
  "command": "restart",
  "params": { "delay": 5000 }
}
```

#### 6. Nettoyage des données
```
DELETE /api/data?device_id=OSW_DEVICE_001
```

## Configuration de la montre

### 1. URL du serveur
Dans l'application "Comm Test" sur la montre, configurez l'URL du serveur :
```
http://VOTRE_IP:8080/api
```

### 2. Configuration WiFi
Assurez-vous que la montre est connectée au même réseau WiFi que le serveur.

### 3. Test de communication
1. Ouvrez l'application "Comm Test" sur la montre
2. Allez dans l'écran "Configuration"
3. Vérifiez que l'URL du serveur est correcte
4. Testez l'envoi de données via l'écran "Test"

## Monitoring en temps réel

### Logs du serveur
Le serveur affiche automatiquement :
- Toutes les requêtes reçues
- Les données des capteurs
- Les statistiques toutes les 30 secondes

### Exemple de sortie
```
🚀 Open-SmartWatch Communication Server running on port 8080
📡 Endpoints:
   GET  / - Server status
   POST /api/data - Receive sensor data
   GET  /api/data - Get stored data
   GET  /api/devices - Get device status
   POST /api/command - Send command to device
   DELETE /api/data - Clear data

🔗 Server URL: http://localhost:8080

[2025-01-27T10:30:15.123Z] Data received from OSW_DEVICE_001:
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
    "battery_level": 85
  }
}

📊 [2025-01-27T10:30:45.123Z] Server Statistics:
   Total messages received: 15
   Active devices: 1
   OSW_DEVICE_001: 15 messages, last seen 0s ago
```

## Test avec curl

### Envoi de données de test
```bash
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -H "Device-ID: OSW_TEST_001" \
  -d '{
    "message_id": "TEST_001",
    "device_id": "OSW_TEST_001",
    "timestamp": 1234567890,
    "type": 0,
    "sensor_data": {
      "temperature": 25.6,
      "battery_level": 85
    }
  }'
```

### Récupération des données
```bash
curl http://localhost:8080/api/data
```

### Statut des devices
```bash
curl http://localhost:8080/api/devices
```

## Personnalisation

### Modification du port
Changez la variable `port` dans `test_server.js` :
```javascript
const port = 3000; // Au lieu de 8080
```

### Limitation du stockage
Modifiez la limite de stockage dans `test_server.js` :
```javascript
if (receivedData.length > 500) { // Au lieu de 1000
    receivedData = receivedData.slice(-500);
}
```

### Ajout de nouveaux endpoints
Ajoutez vos propres routes dans `test_server.js` :
```javascript
app.get('/api/custom', (req, res) => {
    res.json({ message: 'Custom endpoint' });
});
```

## Dépannage

### Problèmes de connexion
1. Vérifiez que le serveur est démarré
2. Vérifiez l'URL dans la configuration de la montre
3. Vérifiez la connectivité réseau
4. Vérifiez les logs du serveur

### Problèmes de données
1. Vérifiez le format JSON envoyé
2. Vérifiez les headers HTTP
3. Vérifiez les logs pour les erreurs

### Performance
- Le serveur stocke les 1000 derniers messages par défaut
- Les statistiques sont affichées toutes les 30 secondes
- Utilisez `npm run dev` pour le développement avec rechargement automatique

## Licence
MIT - Même licence que le projet Open-SmartWatch principal.
