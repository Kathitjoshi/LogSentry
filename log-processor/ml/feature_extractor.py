from datetime import datetime
import hashlib

class FeatureExtractor:
    LOG_LEVEL_MAP = {
        'DEBUG': 0,
        'INFO': 1,
        'WARNING': 2,
        'ERROR': 3,
        'CRITICAL': 4
    }
    
    def extract(self, log_entry):
        features = []
        
        level = log_entry.get('level', 'INFO')
        features.append(self.LOG_LEVEL_MAP.get(level, 1))
        
        message = log_entry.get('message', '')
        features.append(len(message))
        
        timestamp = log_entry.get('timestamp', datetime.now().isoformat())
        try:
            dt = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
            features.append(dt.hour)
        except:
            features.append(12)
        
        try:
            dt = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
            features.append(dt.weekday())
        except:
            features.append(0)
        
        service = log_entry.get('service', 'unknown')
        service_hash = int(hashlib.md5(service.encode()).hexdigest()[:8], 16) % 1000
        features.append(service_hash)
        
        error_keywords = ['error', 'exception', 'failed', 'timeout', 'refused']
        message_lower = message.lower()
        error_count = sum(1 for keyword in error_keywords if keyword in message_lower)
        features.append(error_count)
        
        import re
        number_count = len(re.findall(r'\d+', message))
        features.append(number_count)
        
        metadata = log_entry.get('metadata', {})
        features.append(len(metadata))
        
        return features
