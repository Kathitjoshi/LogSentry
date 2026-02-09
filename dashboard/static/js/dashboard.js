let logLevelChart, anomalyChart;

async function fetchData(endpoint) {
    try {
        const response = await fetch(`/api/${endpoint}`);
        return await response.json();
    } catch (error) {
        console.error(`Error fetching ${endpoint}:`, error);
        return [];
    }
}

function initCharts() {
    const ctx1 = document.getElementById('logLevelChart').getContext('2d');
    logLevelChart = new Chart(ctx1, {
        type: 'bar',
        data: {
            labels: ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
            datasets: [{
                label: 'Log Count',
                data: [0, 0, 0, 0, 0],
                backgroundColor: [
                    'rgba(54, 162, 235, 0.8)',
                    'rgba(75, 192, 192, 0.8)',
                    'rgba(255, 206, 86, 0.8)',
                    'rgba(255, 99, 132, 0.8)',
                    'rgba(153, 102, 255, 0.8)'
                ]
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false
        }
    });
    
    const ctx2 = document.getElementById('anomalyChart').getContext('2d');
    anomalyChart = new Chart(ctx2, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Anomalies',
                data: [],
                borderColor: 'rgb(231, 76, 60)',
                backgroundColor: 'rgba(231, 76, 60, 0.1)',
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false
        }
    });
}

async function updateDashboard() {
    const logs = await fetchData('logs');
    const anomalies = await fetchData('anomalies');
    const stats = await fetchData('stats');
    
    // Update stats
    document.getElementById('total-logs').textContent = stats.num_keys || 0;
    document.getElementById('total-anomalies').textContent = anomalies.length;
    
    // Calculate cache hit rate from total logs and anomalies
    const totalEvents = (stats.num_keys || 0) + (anomalies.length || 0);
    const hitRate = totalEvents > 0 ? (anomalies.length / totalEvents * 100) : 0;
    document.getElementById('cache-hit-rate').textContent = hitRate.toFixed(1) + '%';
    document.getElementById('storage-size').textContent = (stats.total_size_mb || 0).toFixed(2) + ' MB';
    
    // Update log level chart
    const levelCounts = { DEBUG: 0, INFO: 0, WARNING: 0, ERROR: 0, CRITICAL: 0 };
    logs.forEach(log => {
        const level = log.level || 'INFO';
        if (levelCounts.hasOwnProperty(level)) {
            levelCounts[level]++;
        }
    });
    logLevelChart.data.datasets[0].data = Object.values(levelCounts);
    logLevelChart.update();
    
    // Update anomaly chart
    const anomalyTimeline = {};
    anomalies.forEach(anomaly => {
        const time = new Date(anomaly.detected_at).toLocaleTimeString();
        anomalyTimeline[time] = (anomalyTimeline[time] || 0) + 1;
    });
    anomalyChart.data.labels = Object.keys(anomalyTimeline);
    anomalyChart.data.datasets[0].data = Object.values(anomalyTimeline);
    anomalyChart.update();
    
    // Update logs table
    const logsTbody = document.getElementById('logs-tbody');
    logsTbody.innerHTML = '';
    logs.slice(0, 20).forEach(log => {
        const row = logsTbody.insertRow();
        row.innerHTML = `
            <td>${new Date(log.timestamp).toLocaleString()}</td>
            <td class="level-${log.level}">${log.level}</td>
            <td>${log.service}</td>
            <td>${log.message}</td>
        `;
    });
    
    // Update anomalies table
    const anomaliesTbody = document.getElementById('anomalies-tbody');
    anomaliesTbody.innerHTML = '';
    anomalies.slice(0, 10).forEach(anomaly => {
        const row = anomaliesTbody.insertRow();
        row.innerHTML = `
            <td>${new Date(anomaly.detected_at).toLocaleString()}</td>
            <td>${anomaly.log.service}</td>
            <td>${anomaly.log.message}</td>
            <td>${anomaly.score.toFixed(3)}</td>
        `;
    });
}

// Initialize
window.addEventListener('load', () => {
    initCharts();
    updateDashboard();
    setInterval(updateDashboard, 5000);
});
