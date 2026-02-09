#!/usr/bin/env python3
import os, sys, time, json, random, pika
from datetime import datetime

class WebServerLogProducer:
    HTTP_METHODS = ['GET', 'POST', 'PUT', 'DELETE', 'PATCH']
    HTTP_PATHS = ['/api/users', '/api/products', '/api/orders', '/api/auth/login', '/api/auth/logout', '/health', '/metrics', '/admin/dashboard']
    HTTP_STATUS = [200, 200, 200, 201, 204, 400, 401, 403, 404, 500, 502, 503]
    
    def __init__(self, app_name='web-server'):
        self.app_name = app_name
        credentials = pika.PlainCredentials(
            os.getenv('RABBITMQ_USER', 'admin'),
            os.getenv('RABBITMQ_PASS', 'admin123')
        )
        parameters = pika.ConnectionParameters(
            host=os.getenv('RABBITMQ_HOST', 'localhost'),
            port=int(os.getenv('RABBITMQ_PORT', 5672)),
            credentials=credentials
        )
        self.connection = pika.BlockingConnection(parameters)
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue='logs', durable=True)
        print(f"[{self.app_name}] Connected to RabbitMQ")
    
    def generate_log(self):
        method = random.choice(self.HTTP_METHODS)
        path = random.choice(self.HTTP_PATHS)
        status = random.choice(self.HTTP_STATUS)
        response_time = random.randint(10, 500)
        
        if random.random() < 0.05:
            status = random.choice([500, 502, 503, 504])
            response_time = random.randint(5000, 10000)
        
        level = 'INFO'
        if status >= 400:
            level = 'ERROR' if status >= 500 else 'WARNING'
        
        return {
            'timestamp': datetime.now().isoformat(),
            'level': level,
            'service': self.app_name,
            'message': f'{method} {path} - {status} - {response_time}ms',
            'metadata': {
                'method': method, 'path': path,
                'status_code': status, 'response_time_ms': response_time,
                'ip': f'192.168.1.{random.randint(1, 254)}'
            }
        }
    
    def send_log(self, log_entry):
        self.channel.basic_publish(
            exchange='', routing_key='logs',
            body=json.dumps(log_entry),
            properties=pika.BasicProperties(delivery_mode=2)
        )
    
    def run(self, interval=1.0):
        print(f"[{self.app_name}] Starting log generation")
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
    producer = WebServerLogProducer(os.getenv('APP_NAME', 'web-server'))
    producer.run(interval=0.5)
