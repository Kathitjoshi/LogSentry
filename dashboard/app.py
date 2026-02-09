#!/usr/bin/env python3
from flask import Flask, render_template, jsonify
from flask_cors import CORS
import yaml
import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'log-processor', 'src'))
from kvstore_client import KVStoreClient
import json
from datetime import datetime, timedelta

app = Flask(__name__)
CORS(app)

def find_config():
    """Find config file"""
    paths = [
        'config/config.yaml',
        '../config/config.yaml',
        os.path.join(os.path.dirname(__file__), '../config/config.yaml'),
        '/app/config/config.yaml'
    ]
    for p in paths:
        if os.path.exists(p):
            return p
    return None

config_path = find_config()
if config_path:
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
else:
    config = {
        'dashboard': {'secret_key': 'dev-secret-key', 'host': '0.0.0.0', 'port': 5000},
        'kvstore': {'host': 'localhost', 'port': 8080}
    }

app.config['SECRET_KEY'] = config['dashboard']['secret_key']

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/logs')
def get_logs():
    try:
        kvstore = KVStoreClient()  # No host/port needed
        results = kvstore.scan("log:", "log:~", limit=100)
        
        logs = []
        for key, value in results:
            try:
                log_data = json.loads(value)
                logs.append(log_data)
            except:
                continue
        
        kvstore.close()
        
        # Return mock data if no logs found
        if not logs:
            return jsonify([
                {
                    'timestamp': datetime.now().isoformat(),
                    'level': 'INFO',
                    'service': 'web-server',
                    'message': 'GET /api/users - 200 - 45ms'
                }
            ])
        
        return jsonify(logs)
    except Exception as e:
        print(f"Error in get_logs: {e}")
        # Return mock data if error
        return jsonify([
            {
                'timestamp': datetime.now().isoformat(),
                'level': 'INFO',
                'service': 'web-server',
                'message': 'Waiting for logs...'
            }
        ])

@app.route('/api/anomalies')
def get_anomalies():
    try:
        kvstore = KVStoreClient()  # No host/port needed
        results = kvstore.scan("anomaly:", "anomaly:~", limit=50)
        
        anomalies = []
        for key, value in results:
            try:
                anomaly_data = json.loads(value)
                anomalies.append(anomaly_data)
            except:
                continue
        
        kvstore.close()
        return jsonify(anomalies)
    except Exception as e:
        print(f"Error in get_anomalies: {e}")
        return jsonify([])

@app.route('/api/stats')
def get_stats():
    try:
        kvstore = KVStoreClient()  # No host/port needed
        stats = kvstore.get_stats()
        kvstore.close()
        return jsonify(stats)
    except Exception as e:
        print(f"Error in get_stats: {e}")
        return jsonify({
            'num_keys': 0,
            'total_size_bytes': 0,
            'total_size_mb': 0.0
        })

if __name__ == '__main__':
    app.run(
        host=config['dashboard']['host'],
        port=config['dashboard']['port'],
        debug=False
    )
