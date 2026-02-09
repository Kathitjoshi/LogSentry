import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import joblib
import os
from feature_extractor import FeatureExtractor

class AnomalyDetector:
    def __init__(self, contamination=0.1, n_estimators=100, model_path='ml/models'):
        self.contamination = contamination
        self.n_estimators = n_estimators
        self.model_path = model_path
        
        self.feature_extractor = FeatureExtractor()
        self.scaler = StandardScaler()
        self.model = IsolationForest(
            contamination=contamination,
            n_estimators=n_estimators,
            random_state=42
        )
        
        self.is_trained = False
        self._load_model()
    
    def train(self, log_data):
        if len(log_data) < 10:
            return
        
        features = [self.feature_extractor.extract(log) for log in log_data]
        X = np.array(features)
        
        X_scaled = self.scaler.fit_transform(X)
        self.model.fit(X_scaled)
        self.is_trained = True
        
        self._save_model()
    
    def detect(self, log_entry):
        if not self.is_trained:
            return False
        
        features = self.feature_extractor.extract(log_entry)
        X = np.array([features])
        X_scaled = self.scaler.transform(X)
        
        prediction = self.model.predict(X_scaled)
        return prediction[0] == -1
    
    def detect_batch(self, log_entries):
        if not self.is_trained:
            self.train(log_entries)
            return [False] * len(log_entries)
        
        features = [self.feature_extractor.extract(log) for log in log_entries]
        X = np.array(features)
        X_scaled = self.scaler.transform(X)
        
        predictions = self.model.predict(X_scaled)
        return [pred == -1 for pred in predictions]
    
    def get_anomaly_score(self, log_entry):
        if not self.is_trained:
            return 0.0
        
        features = self.feature_extractor.extract(log_entry)
        X = np.array([features])
        X_scaled = self.scaler.transform(X)
        
        score = self.model.score_samples(X_scaled)
        return float(score[0])
    
    def _save_model(self):
        os.makedirs(self.model_path, exist_ok=True)
        joblib.dump(self.model, os.path.join(self.model_path, 'model.pkl'))
        joblib.dump(self.scaler, os.path.join(self.model_path, 'scaler.pkl'))
    
    def _load_model(self):
        model_file = os.path.join(self.model_path, 'model.pkl')
        scaler_file = os.path.join(self.model_path, 'scaler.pkl')
        
        if os.path.exists(model_file) and os.path.exists(scaler_file):
            self.model = joblib.load(model_file)
            self.scaler = joblib.load(scaler_file)
            self.is_trained = True
