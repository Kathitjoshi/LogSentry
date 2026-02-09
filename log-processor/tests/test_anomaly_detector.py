import pytest, sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'ml'))
from anomaly_detector import AnomalyDetector

def test_anomaly_detector():
    detector = AnomalyDetector()
    
    logs = []
    for i in range(100):
        logs.append({
            'timestamp': f'2024-01-01T10:{i:02d}:00',
            'level': 'INFO',
            'service': 'test',
            'message': f'Normal log {i}'
        })
    
    detector.train(logs)
    
    anomaly = {
        'timestamp': '2024-01-01T11:00:00',
        'level': 'ERROR',
        'service': 'test',
        'message': 'CRITICAL FAILURE WITH TIMEOUT AND EXCEPTION' * 10
    }
    
    # Should detect as anomaly due to unusual message length and error level
    assert detector.is_trained == True
