// static/js/app.js
document.addEventListener('DOMContentLoaded', function() {
    // Initialize map
    const map = L.map('map').setView([0, 0], 2);
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);
    
    // Create drone marker and path layer
    const droneIcon = L.icon({
        iconUrl: 'https://cdn-icons-png.flaticon.com/512/1829/1829515.png',
        iconSize: [32, 32],
        iconAnchor: [16, 16]
    });
    
    let droneMarker = L.marker([0, 0], {icon: droneIcon}).addTo(map);
    let pathLayer = L.layerGroup().addTo(map);
    let pathPoints = [];
    
    // Connect to Socket.IO server
    const socket = io();
    
    // Connection status
    const connectionStatus = document.getElementById('connection-status');
    const connectionDot = connectionStatus.querySelector('.connection-dot');
    
    socket.on('connect', () => {
        connectionDot.classList.remove('disconnected');
        connectionDot.classList.add('connected');
        connectionStatus.querySelector('span').textContent = 'Connected to server';
    });
    
    socket.on('disconnect', () => {
        connectionDot.classList.remove('connected');
        connectionDot.classList.add('disconnected');
        connectionStatus.querySelector('span').textContent = 'Disconnected';
    });
    
    // Handle incoming data
    socket.on('update_data', function(data) {
        updateUI(data.current);
        updateMap(data.current, data.history);
    });
    
    // Initial data load
    fetch('/data')
        .then(response => response.json())
        .then(data => {
            updateUI(data.current);
            updateMap(data.current, data.history);
        });
    
    function updateUI(data) {
        // Update sensor values
        document.getElementById('distance-value').textContent = data.distance.toFixed(1);
        document.getElementById('temperature-value').textContent = data.temperature.toFixed(1);
        document.getElementById('humidity-value').textContent = data.humidity.toFixed(1);
        document.getElementById('altitude-value').textContent = data.altitude.toFixed(1);
        
        // Update GPS values
        const gpsStatus = document.getElementById('gps-status');
        if (data.gps_fix) {
            gpsStatus.className = 'status-indicator status-good';
            document.getElementById('latitude-value').textContent = data.latitude.toFixed(7);
            document.getElementById('longitude-value').textContent = data.longitude.toFixed(7);
        } else {
            gpsStatus.className = 'status-indicator status-bad';
            document.getElementById('latitude-value').textContent = '--';
            document.getElementById('longitude-value').textContent = '--';
        }
        
        // Update heartbeat
        const heartbeatStatus = document.getElementById('heartbeat-status');
        heartbeatStatus.className = data.heartbeat ? 
            'status-indicator status-good' : 'status-indicator status-bad';
        
        // Update timestamp
        const now = new Date();
        document.getElementById('last-update').textContent = 
            `Last update: ${now.toLocaleTimeString()}`;
    }
    
    function updateMap(currentData, history) {
        if (currentData.gps_fix && currentData.latitude !== 0 && currentData.longitude !== 0) {
            // Update drone marker position
            const newPos = [currentData.latitude, currentData.longitude];
            droneMarker.setLatLng(newPos);
            
            // Add to path
            pathPoints.push(newPos);
            
            // Update path layer
            pathLayer.clearLayers();
            if (pathPoints.length > 1) {
                L.polyline(pathPoints, {color: '#3498db', weight: 3}).addTo(pathLayer);
            }
            
            // Center map on drone if it's the first fix
            if (pathPoints.length === 1) {
                map.setView(newPos, 15);
            }
        }
        
        // If we have history, plot it
        if (history && history.length > 0) {
            pathPoints = history.map(point => [point.lat, point.lng]);
            pathLayer.clearLayers();
            if (pathPoints.length > 1) {
                L.polyline(pathPoints, {color: '#3498db', weight: 3}).addTo(pathLayer);
            }
            
            // Set view to last known position
            const lastPoint = pathPoints[pathPoints.length - 1];
            droneMarker.setLatLng(lastPoint);
            map.setView(lastPoint, 15);
        }
    }
});