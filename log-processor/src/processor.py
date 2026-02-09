#!/usr/bin/env python3
import pika
import json
import os
import sys
import logging
import yaml
from datetime import datetime
import time

# Add paths
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'ml'))

from kvstore_client import KVStoreClient
from log_parser import LogParser
from anomaly_detector import AnomalyDetector

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class LogProcessor:
    def __init__(self, config):
        self.config = config
        
        # Initialize KV Store (file-based, no host/port needed)
        logger.info("Initializing KV store...")
        self.kvstore = KVStoreClient()
        
        # Initialize parser and anomaly detector
        self.parser = LogParser()
        self.anomaly_detector = AnomalyDetector()
        
        # Stats
        self.processed_count = 0
        self.anomaly_count = 0
        
        logger.info("LogProcessor initialized")
    
    def callback(self, ch, method, properties, body):
        """Process incoming log message"""
        try:
            # Decode message
            log_data = json.loads(body.decode())
            logger.info(f"Processing log: {log_data.get('message', 'Unknown')[:50]}...")
            
            # Parse log
            parsed_log = self.parser.parse(log_data)
            
            # Check for anomaly
            is_anomaly = self.anomaly_detector.detect(parsed_log)
            
            # Store log in KV store
            timestamp = log_data.get('timestamp', datetime.now().isoformat())
            service = log_data.get('service', 'unknown')
            log_key = f"log:{service}:{timestamp}"
            
            success = self.kvstore.put(log_key, json.dumps(log_data))
            
            if success:
                self.processed_count += 1
                logger.info(f"✅ Stored log #{self.processed_count}: {log_key}")
            else:
                logger.error(f"❌ Failed to store log: {log_key}")
            
            # Store anomaly if detected
            if is_anomaly:
                self.anomaly_count += 1
                anomaly_key = f"anomaly:{service}:{timestamp}"
                anomaly_data = {
                    **log_data,
                    'detected_at': datetime.now().isoformat(),
                    'anomaly_score': parsed_log.get('anomaly_score', 0)
                }
                self.kvstore.put(anomaly_key, json.dumps(anomaly_data))
                logger.warning(f"🚨 Anomaly detected #{self.anomaly_count}: {anomaly_key}")
            
            # Acknowledge message
            ch.basic_ack(delivery_tag=method.delivery_tag)
            
            # Log stats every 10 messages
            if self.processed_count % 10 == 0:
                logger.info(f"📊 Stats: {self.processed_count} logs processed, {self.anomaly_count} anomalies detected")
        
        except Exception as e:
            logger.error(f"Error processing message: {e}", exc_info=True)
            ch.basic_nack(delivery_tag=method.delivery_tag, requeue=False)
    
    def start(self):
        """Start consuming messages from RabbitMQ"""
        max_retries = 10
        retry_delay = 5
        
        for attempt in range(max_retries):
            try:
                # Get RabbitMQ config - handle both 'username' and 'user' fields
                rmq_config = self.config.get('rabbitmq', {})
                
                rabbitmq_host = os.getenv('RABBITMQ_HOST', rmq_config.get('host', 'rabbitmq'))
                rabbitmq_port = int(os.getenv('RABBITMQ_PORT', rmq_config.get('port', 5672)))
                # Try 'username' first (from config.yaml), then 'user', then default
                rabbitmq_user = os.getenv('RABBITMQ_USER', 
                    rmq_config.get('username', rmq_config.get('user', 'admin')))
                rabbitmq_pass = os.getenv('RABBITMQ_PASS', 
                    rmq_config.get('password', 'admin123'))
                
                logger.info(f"Connecting to RabbitMQ: {rabbitmq_host}:{rabbitmq_port} as {rabbitmq_user}")
                
                credentials = pika.PlainCredentials(rabbitmq_user, rabbitmq_pass)
                parameters = pika.ConnectionParameters(
                    host=rabbitmq_host,
                    port=rabbitmq_port,
                    credentials=credentials,
                    heartbeat=600,
                    blocked_connection_timeout=300
                )
                
                # Connect
                self.connection = pika.BlockingConnection(parameters)
                self.channel = self.connection.channel()
                
                # Declare queue
                self.channel.queue_declare(queue='logs', durable=True)
                
                # Set QoS
                self.channel.basic_qos(prefetch_count=1)
                
                # Start consuming
                self.channel.basic_consume(
                    queue='logs',
                    on_message_callback=self.callback
                )
                
                logger.info(f"✅ Connected to RabbitMQ at {rabbitmq_host}:{rabbitmq_port}")
                logger.info("Waiting for messages...")
                logger.info("=" * 60)
                
                self.channel.start_consuming()
                
            except pika.exceptions.AMQPConnectionError as e:
                logger.error(f"Connection error: {e}")
                if attempt < max_retries - 1:
                    logger.info(f"Reconnecting in {retry_delay} seconds...")
                    time.sleep(retry_delay)
                else:
                    logger.error("Max retries reached. Exiting.")
                    raise
            except KeyboardInterrupt:
                logger.info("Shutting down...")
                self.stop()
                break
            except Exception as e:
                logger.error(f"Unexpected error: {e}", exc_info=True)
                if attempt < max_retries - 1:
                    time.sleep(retry_delay)
                else:
                    raise
    
    def stop(self):
        """Stop consuming and close connections"""
        try:
            if hasattr(self, 'channel') and self.channel:
                self.channel.stop_consuming()
            if hasattr(self, 'connection') and self.connection:
                self.connection.close()
            if hasattr(self, 'kvstore') and self.kvstore:
                self.kvstore.close()
            logger.info(f"Final stats: {self.processed_count} logs, {self.anomaly_count} anomalies")
        except Exception as e:
            logger.error(f"Error during shutdown: {e}")

def find_config():
    """Find config file"""
    paths = [
        'config/config.yaml',
        '../config/config.yaml',
        '/app/config/config.yaml',
        os.path.join(os.path.dirname(__file__), '../../config/config.yaml')
    ]
    for p in paths:
        if os.path.exists(p):
            return p
    return None

if __name__ == '__main__':
    # Load config
    config_path = find_config()
    
    if config_path:
        logger.info(f"Found config file at: {config_path}")
        with open(config_path, 'r') as f:
            config = yaml.safe_load(f)
    else:
        logger.warning("No config file found, using defaults")
        config = {
            'rabbitmq': {
                'host': 'localhost',
                'port': 5672,
                'username': 'admin',
                'password': 'admin123'
            }
        }
    
    # Start processor
    logger.info("Starting log processor...")
    processor = LogProcessor(config)
    processor.start()