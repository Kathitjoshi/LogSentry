import pytest
import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))
from log_parser import LogParser

def test_parse_dict():
    parser = LogParser()
    log_data = {
        'timestamp': '2024-01-01T10:00:00',
        'level': 'INFO',
        'service': 'test',
        'message': 'Test message'
    }
    result = parser.parse(log_data)
    assert result['level'] == 'INFO'
    assert result['service'] == 'test'

def test_parse_string():
    parser = LogParser()
    log_string = '{"timestamp": "2024-01-01T10:00:00", "level": "ERROR", "message": "Error occurred"}'
    result = parser.parse(log_string)
    assert result['level'] == 'ERROR'
