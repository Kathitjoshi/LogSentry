#!/usr/bin/env python3
import os, time, json, random, pika
from datetime import datetime

class APIServerLogProducer:
    API_ENDPOINTS = ['/v1/users', '/v1/products', '/v1/orders', '/v1/payments', '/v1/inventory', '/v1/analytics']
    
    def __init__(self, app_name='api-server'):
        self.app_name = app_name
        credentials = pika.PlainCredentials(os.getenv('RABBITMQ_USER', 'admin'), os.getenv('RABBITMQ_PASS', 'admin123'))
        parameters = pika.ConnectionParameters(host=os.getenv('RABBITMQ_HOST', 'localhost'), port=int(os.getenv('RABBITMQ_PORT', 5672)), credentials=credentials)
        self.connection = pika.BlockingConnection(parameters)
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue='logs', durable=True)
        print(f"[{self.app_name}] Connected")
    
    def generate_log(self):
        endpoint = random.choice(self.API_ENDPOINTS)
        duration = random.randint(50, 300)
        success = random.random() > 0.1
        
        if not success:
            duration = random.randint(1000, 5000)
        
        return {
            'timestamp': datetime.now().isoformat(),
            'level': 'INFO' if success else 'ERROR',
            'service': self.app_name,
            'message': f'API request {endpoint} completed in {duration}ms',
            'metadata': {'endpoint': endpoint, 'duration_ms': duration, 'success': success}
        }
    
    def send_log(self, log_entry):
        self.channel.basic_publish(exchange='', routing_key='logs', body=json.dumps(log_entry), properties=pika.BasicProperties(delivery_mode=2))
    
    def run(self, interval=0.8):
        print(f"[{self.app_name}] Starting")
        try:
            while True:
                log_entry = self.generate_log()
                self.send_log(log_entry)
                print(f"[{self.app_name}] {log_entry['level']}: {log_entry['message']}")
                time.sleep(interval + random.uniform(-0.2, 0.2))
        except KeyboardInterrupt:
            print(f"\n[{self.app_name}] Stopping...")
        finally:
            self.connection.close()

if __name__ == '__main__':
    producer = APIServerLogProducer(os.getenv('APP_NAME', 'api-server'))
    producer.run()
