#!/usr/bin/env python3
import os, time, json, random, pika
from datetime import datetime

class DatabaseLogProducer:
    def __init__(self, app_name='database'):
        self.app_name = app_name
        credentials = pika.PlainCredentials(os.getenv('RABBITMQ_USER', 'admin'), os.getenv('RABBITMQ_PASS', 'admin123'))
        parameters = pika.ConnectionParameters(host=os.getenv('RABBITMQ_HOST', 'localhost'), port=int(os.getenv('RABBITMQ_PORT', 5672)), credentials=credentials)
        self.connection = pika.BlockingConnection(parameters)
        self.channel = self.connection.channel()
        self.channel.queue_declare(queue='logs', durable=True)
        print(f"[{self.app_name}] Connected")
    
    def generate_log(self):
        query_type = random.choice(['SELECT', 'INSERT', 'UPDATE', 'DELETE'])
        duration = random.randint(5, 200)
        if random.random() < 0.03:
            duration = random.randint(2000, 8000)
        return {
            'timestamp': datetime.now().isoformat(),
            'level': 'INFO' if duration < 1000 else 'WARNING',
            'service': self.app_name,
            'message': f'{query_type} query executed in {duration}ms',
            'metadata': {'query_type': query_type, 'duration_ms': duration, 'rows': random.randint(1, 1000)}
        }
    
    def send_log(self, log_entry):
        self.channel.basic_publish(exchange='', routing_key='logs', body=json.dumps(log_entry), properties=pika.BasicProperties(delivery_mode=2))
    
    def run(self, interval=1.5):
        print(f"[{self.app_name}] Starting")
        try:
            while True:
                log_entry = self.generate_log()
                self.send_log(log_entry)
                print(f"[{self.app_name}] {log_entry['level']}: {log_entry['message']}")
                time.sleep(interval + random.uniform(-0.3, 0.3))
        except KeyboardInterrupt:
            print(f"\n[{self.app_name}] Stopping...")
        finally:
            self.connection.close()

if __name__ == '__main__':
    producer = DatabaseLogProducer(os.getenv('APP_NAME', 'database'))
    producer.run()
