#!/usr/bin/env python3
import os, sys, time, yaml, logging, json
from datetime import datetime
sys.path.append(os.path.dirname(__file__))
from kvstore_client import KVStoreClient
from notifications import send_email, send_webhook

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class AlertManager:
    def __init__(self, config_path='config/config.yaml'):
        with open(config_path, 'r') as f:
            self.config = yaml.safe_load(f)
        
        kvstore_host = os.getenv('KVSTORE_HOST', self.config['kvstore']['host'])
        kvstore_port = int(os.getenv('KVSTORE_PORT', self.config['kvstore']['port']))
        self.kvstore = KVStoreClient(kvstore_host, kvstore_port)
        
        self.alert_config = self.config['alerting']
        self.last_alert_time = {}
        
        logger.info("AlertManager initialized")
    
    def check_anomalies(self):
        results = self.kvstore.scan("anomaly:", "anomaly:~", limit=100)
        
        for key, value in results:
            try:
                anomaly_data = json.loads(value)
                self.process_anomaly(anomaly_data)
            except Exception as e:
                logger.error(f"Error processing anomaly: {e}")
    
    def process_anomaly(self, anomaly_data):
        log_entry = anomaly_data['log']
        score = anomaly_data['score']
        
        if abs(score) < self.alert_config['thresholds']['anomaly_score']:
            return
        
        service = log_entry.get('service', 'unknown')
        cooldown = self.alert_config['thresholds']['alert_cooldown_seconds']
        
        if service in self.last_alert_time:
            time_since_last = time.time() - self.last_alert_time[service]
            if time_since_last < cooldown:
                return
        
        self.send_alert(anomaly_data)
        self.last_alert_time[service] = time.time()
    
    def send_alert(self, anomaly_data):
        message = f"Anomaly detected in {anomaly_data['log']['service']}: {anomaly_data['log']['message']}"
        
        if self.alert_config['email']['enabled']:
            send_email(self.alert_config['email'], "Log Anomaly Detected", message)
        
        if self.alert_config['webhook']['enabled']:
            send_webhook(self.alert_config['webhook']['url'], message)
        
        logger.warning(f"Alert sent: {message}")
    
    def run(self):
        logger.info("Starting alert manager...")
        while True:
            try:
                self.check_anomalies()
                time.sleep(30)
            except KeyboardInterrupt:
                break
            except Exception as e:
                logger.error(f"Error: {e}")
                time.sleep(5)

if __name__ == '__main__':
    manager = AlertManager()
    manager.run()
