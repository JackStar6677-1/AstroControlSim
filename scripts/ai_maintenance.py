import asyncio
import websockets
import json
import numpy as np

WS_URL = "ws://localhost:9001"

class JackstarAI:
    def __init__(self):
        self.health_history = {} # id -> [last_temperatures]
        self.threshold_z = 3.0 # Standard deviations for anomaly
        print("Jackstar AI Maintenance Active: Monitoring Array Health...")

    async def analyze(self):
        async with websockets.connect(WS_URL) as ws:
            while True:
                msg = await ws.recv()
                data = json.loads(msg)
                
                for ant in data:
                    ant_id = ant['id']
                    temp = ant['temp']
                    current = ant['current']
                    
                    if ant_id not in self.health_history:
                        self.health_history[ant_id] = []
                    
                    self.health_history[ant_id].append(temp)
                    
                    # Keep last 50 samples for baseline
                    if len(self.health_history[ant_id]) > 50:
                        history = np.array(self.health_history[ant_id])
                        mean = np.mean(history)
                        std = np.std(history)
                        
                        # Anomaly Detection (Z-Score)
                        if std > 0.1: # Avoid division by zero in stable state
                            z_score = abs(temp - mean) / std
                            if z_score > self.threshold_z:
                                print(f"⚠️  [AI ADVISORY] ANT-{ant_id:02d}: Abnormal Heat Signature (Z={z_score:.2f})")
                        
                        # High Load warning
                        if current > 8.0:
                             print(f"⚡ [AI WARNING] ANT-{ant_id:02d}: Excessive Motor Current ({current:.1f}A)")

                        self.health_history[ant_id].pop(0)

                await asyncio.sleep(0.01)

if __name__ == "__main__":
    ai = JackstarAI()
    try:
        asyncio.run(ai.analyze())
    except Exception as e:
        print(f"AI Disconnected: {e}")
