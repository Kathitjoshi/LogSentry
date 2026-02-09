#!/usr/bin/env python3
import os
import json
import logging
from pathlib import Path

logger = logging.getLogger(__name__)

class KVStoreClient:
    """Simple file-based key-value store (no server needed)"""
    
    def __init__(self, host=None, port=None):
        # Ignore host/port - we're using files instead
        self.data_dir = Path('/app/data/kvstore')
        self.data_dir.mkdir(parents=True, exist_ok=True)
        logger.info(f"Using file-based KV store at {self.data_dir}")
    
    def put(self, key, value):
        """Store key-value pair as a file"""
        try:
            # Sanitize key for filename
            safe_key = key.replace('/', '_').replace(':', '_')
            file_path = self.data_dir / f"{safe_key}.json"
            
            with open(file_path, 'w') as f:
                json.dump({'key': key, 'value': value}, f)
            
            return True
        except Exception as e:
            logger.error(f"Error storing {key}: {e}")
            return False
    
    def get(self, key):
        """Retrieve value by key"""
        try:
            safe_key = key.replace('/', '_').replace(':', '_')
            file_path = self.data_dir / f"{safe_key}.json"
            
            if file_path.exists():
                with open(file_path, 'r') as f:
                    data = json.load(f)
                    return data.get('value')
            return None
        except Exception as e:
            logger.error(f"Error retrieving {key}: {e}")
            return None
    
    def scan(self, start_key, end_key, limit=100):
        """Scan keys in range (simplified)"""
        try:
            results = []
            for file_path in sorted(self.data_dir.glob('*.json'))[:limit]:
                with open(file_path, 'r') as f:
                    data = json.load(f)
                    key = data.get('key')
                    if start_key <= key < end_key:
                        results.append((key, data.get('value')))
            
            return results
        except Exception as e:
            logger.error(f"Error scanning: {e}")
            return []
    
    def delete(self, key):
        """Delete key"""
        try:
            safe_key = key.replace('/', '_').replace(':', '_')
            file_path = self.data_dir / f"{safe_key}.json"
            
            if file_path.exists():
                file_path.unlink()
                return True
            return False
        except Exception as e:
            logger.error(f"Error deleting {key}: {e}")
            return False
    
    def close(self):
        """No-op for file-based storage"""
        pass
    
    def get_stats(self):
        """Get storage statistics"""
        try:
            num_keys = len(list(self.data_dir.glob('*.json')))
            
            # Calculate total size
            total_size = sum(f.stat().st_size for f in self.data_dir.glob('*.json'))
            
            return {
                'num_keys': num_keys,
                'total_size_bytes': total_size,
                'total_size_mb': round(total_size / (1024 * 1024), 2)
            }
        except Exception as e:
            logger.error(f"Error getting stats: {e}")
            return {'num_keys': 0, 'total_size_bytes': 0, 'total_size_mb': 0}
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
