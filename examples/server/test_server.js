const express = require('express');
const cors = require('cors');
const app = express();
const port = 8080;

// Middleware
app.use(cors());
app.use(express.json());

// Stockage des données reçues
let receivedData = [];
let deviceStatus = {};

// Routes
app.get('/', (req, res) => {
    res.json({
        message: 'Open-SmartWatch Communication Server',
        status: 'running',
        received_messages: receivedData.length,
        devices: Object.keys(deviceStatus)
    });
});

// Endpoint pour recevoir les données
app.post('/api/data', (req, res) => {
    const deviceId = req.headers['device-id'] || 'unknown';
    const timestamp = new Date().toISOString();
    
    console.log(`[${timestamp}] Data received from ${deviceId}:`);
    console.log(JSON.stringify(req.body, null, 2));
    
    // Stocker les données
    const dataEntry = {
        device_id: deviceId,
        timestamp: timestamp,
        data: req.body
    };
    
    receivedData.push(dataEntry);
    
    // Limiter le nombre d'entrées stockées
    if (receivedData.length > 1000) {
        receivedData = receivedData.slice(-1000);
    }
    
    // Mettre à jour le statut du device
    deviceStatus[deviceId] = {
        last_seen: timestamp,
        message_count: (deviceStatus[deviceId]?.message_count || 0) + 1,
        connection_type: req.body.type === 0 ? 'WiFi' : 'Bluetooth'
    };
    
    res.json({
        status: 'success',
        message: 'Data received successfully',
        device_id: deviceId,
        timestamp: timestamp
    });
});

// Endpoint pour récupérer les données
app.get('/api/data', (req, res) => {
    const limit = parseInt(req.query.limit) || 50;
    const deviceId = req.query.device_id;
    
    let filteredData = receivedData;
    if (deviceId) {
        filteredData = receivedData.filter(entry => entry.device_id === deviceId);
    }
    
    res.json({
        data: filteredData.slice(-limit),
        total: filteredData.length,
        devices: deviceStatus
    });
});

// Endpoint pour le statut des devices
app.get('/api/devices', (req, res) => {
    res.json({
        devices: deviceStatus,
        total_devices: Object.keys(deviceStatus).length
    });
});

// Endpoint pour envoyer des commandes
app.post('/api/command', (req, res) => {
    const { device_id, command, params } = req.body;
    
    console.log(`[${new Date().toISOString()}] Command sent to ${device_id}: ${command}`);
    
    res.json({
        status: 'success',
        message: 'Command sent successfully',
        command: command,
        device_id: device_id
    });
});

// Endpoint pour nettoyer les données
app.delete('/api/data', (req, res) => {
    const deviceId = req.query.device_id;
    
    if (deviceId) {
        receivedData = receivedData.filter(entry => entry.device_id !== deviceId);
        delete deviceStatus[deviceId];
    } else {
        receivedData = [];
        deviceStatus = {};
    }
    
    res.json({
        status: 'success',
        message: 'Data cleared successfully'
    });
});

// Middleware pour logger les requêtes
app.use((req, res, next) => {
    console.log(`${new Date().toISOString()} - ${req.method} ${req.path}`);
    next();
});

// Gestion des erreurs
app.use((err, req, res, next) => {
    console.error('Error:', err);
    res.status(500).json({
        status: 'error',
        message: 'Internal server error',
        error: err.message
    });
});

// Démarrage du serveur
app.listen(port, () => {
    console.log(`🚀 Open-SmartWatch Communication Server running on port ${port}`);
    console.log(`📡 Endpoints:`);
    console.log(`   GET  / - Server status`);
    console.log(`   POST /api/data - Receive sensor data`);
    console.log(`   GET  /api/data - Get stored data`);
    console.log(`   GET  /api/devices - Get device status`);
    console.log(`   POST /api/command - Send command to device`);
    console.log(`   DELETE /api/data - Clear data`);
    console.log(`\n🔗 Server URL: http://localhost:${port}`);
});

// Fonction pour afficher les statistiques périodiquement
setInterval(() => {
    const now = new Date();
    console.log(`\n📊 [${now.toISOString()}] Server Statistics:`);
    console.log(`   Total messages received: ${receivedData.length}`);
    console.log(`   Active devices: ${Object.keys(deviceStatus).length}`);
    
    Object.entries(deviceStatus).forEach(([deviceId, status]) => {
        const lastSeen = new Date(status.last_seen);
        const timeDiff = Math.floor((now - lastSeen) / 1000);
        console.log(`   ${deviceId}: ${status.message_count} messages, last seen ${timeDiff}s ago`);
    });
}, 30000); // Toutes les 30 secondes
