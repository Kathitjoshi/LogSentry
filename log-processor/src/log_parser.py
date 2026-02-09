import re
import json
from datetime import datetime

class LogParser:
    PATTERNS = {
        'apache': re.compile(
            r'(?P<ip>\S+) \S+ \S+ \[(?P<timestamp>[^\]]+)\] '
            r'"(?P<method>\S+) (?P<path>\S+) \S+" (?P<status>\d+) (?P<size>\d+)'
        ),
        'nginx': re.compile(
            r'(?P<ip>\S+) - - \[(?P<timestamp>[^\]]+)\] '
            r'"(?P<method>\S+) (?P<path>\S+) \S+" (?P<status>\d+) (?P<size>\d+)'
        ),
        'json': None,
        'syslog': re.compile(
            r'(?P<timestamp>\w+\s+\d+\s+\d+:\d+:\d+) (?P<host>\S+) '
            r'(?P<service>\S+): (?P<message>.*)'
        )
    }
    
    LOG_LEVELS = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL']
    
    def parse(self, log_data):
        if isinstance(log_data, dict):
            return self._parse_dict(log_data)
        elif isinstance(log_data, str):
            return self._parse_string(log_data)
        else:
            raise ValueError("Log data must be dict or string")
    
    def _parse_dict(self, log_dict):
        parsed = {
            'timestamp': log_dict.get('timestamp', datetime.now().isoformat()),
            'level': log_dict.get('level', 'INFO'),
            'service': log_dict.get('service', 'unknown'),
            'message': log_dict.get('message', ''),
            'metadata': log_dict.get('metadata', {})
        }
        
        if 'error' in log_dict:
            parsed['metadata']['error'] = log_dict['error']
        
        return parsed
    
    def _parse_string(self, log_string):
        try:
            return self._parse_dict(json.loads(log_string))
        except:
            pass
        
        for format_name, pattern in self.PATTERNS.items():
            if pattern is None:
                continue
            
            match = pattern.match(log_string)
            if match:
                return self._parse_match(match, format_name)
        
        return self._parse_default(log_string)
    
    def _parse_match(self, match, format_name):
        data = match.groupdict()
        
        parsed = {
            'timestamp': data.get('timestamp', datetime.now().isoformat()),
            'level': self._extract_level(str(data)),
            'service': format_name,
            'message': data.get('message', str(data)),
            'metadata': {k: v for k, v in data.items() 
                        if k not in ['timestamp', 'message']}
        }
        
        return parsed
    
    def _parse_default(self, log_string):
        return {
            'timestamp': datetime.now().isoformat(),
            'level': self._extract_level(log_string),
            'service': 'unknown',
            'message': log_string,
            'metadata': {}
        }
    
    def _extract_level(self, text):
        text_upper = text.upper()
        for level in self.LOG_LEVELS:
            if level in text_upper:
                return level
        return 'INFO'
