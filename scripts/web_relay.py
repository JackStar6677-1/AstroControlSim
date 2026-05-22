import socket
import struct
import asyncio
import websockets
import json

# Jackstar Protocol (86 bytes)
PACKET_FMT = '<BIdddddddddd B' 
PACKET_SIZE = struct.calcsize(PACKET_FMT)

TCP_IP = '127.0.0.1'
TCP_PORT = 9000
WS_PORT = 9001

async def telemetery_relay(websocket):
    print(f"Web Client Connected: {websocket.remote_address}")
    
    # Connect to Jackstar C++ Server
    tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        tcp_sock.connect((TCP_IP, TCP_PORT))
        print("Connected to Jackstar C++ Server")
        
        socket_lock = asyncio.Lock()
        
        async def send_telemetry_loop():
            while True:
                async with socket_lock:
                    # Request Telemetry (CMD_GET_TELEMETRY = 3)
                    # Send 0s for the rest of the 86-byte packet
                    req = struct.pack(PACKET_FMT, 3, 0, 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0, 0)
                    tcp_sock.sendall(req)
                    
                    telemetry_data = []
                    for _ in range(50):
                        data = tcp_sock.recv(PACKET_SIZE)
                        if len(data) < PACKET_SIZE: break
                        
                        p_type, p_id, p_az, p_el, p_ae, p_ee, p_x, p_y, p_sa, p_sp, p_mt, p_mc, p_st = struct.unpack(PACKET_FMT, data)
                        
                        telemetry_data.append({
                            "id": p_id,
                            "az": round(p_az, 3),
                            "el": round(p_el, 3),
                            "error": round((p_ae**2 + p_ee**2)**0.5, 4),
                            "state": p_st,
                            "x": p_x,
                            "y": p_y,
                            "amp": round(p_sa, 2),
                            "phase": round(p_sp, 2),
                            "temp": round(p_mt, 1),
                            "current": round(p_mc, 2)
                        })
                
                # Send to Web Client as JSON
                await websocket.send(json.dumps(telemetry_data))
                await asyncio.sleep(0.1) # 10Hz Refresh rate

        async def recv_commands_loop():
            async for message in websocket:
                cmd = json.loads(message)
                action = cmd.get('action')
                if action == 'move':
                    az = cmd.get('azimuth', 0.0)
                    el = cmd.get('elevation', 0.0)
                    pkt = struct.pack(PACKET_FMT, 1, 0, az, el, 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0, 0)
                    async with socket_lock:
                        tcp_sock.sendall(pkt)
                    print(f"Relayed MOVE command: Az={az}, El={el}")
                elif action == 'reset':
                    pkt = struct.pack(PACKET_FMT, 4, 0, 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0, 0)
                    async with socket_lock:
                        tcp_sock.sendall(pkt)
                    print("Relayed Global RESET command")
                elif action == 'tune_pid':
                    kp = cmd.get('kp', 1.2)
                    ki = cmd.get('ki', 0.05)
                    kd = cmd.get('kd', 0.3)
                    ant_id = cmd.get('antennaId', 0)
                    pkt = struct.pack(PACKET_FMT, 5, ant_id, kp, ki, kd, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0)
                    async with socket_lock:
                        tcp_sock.sendall(pkt)
                    print(f"Relayed PID TUNE command for ANT-{ant_id}: Kp={kp}, Ki={ki}, Kd={kd}")

        await asyncio.gather(
            send_telemetry_loop(),
            recv_commands_loop()
        )
            
    except Exception as e:
        print(f"Relay Error: {e}")
    finally:
        tcp_sock.close()

async def main():
    print(f"Jackstar Web Relay starting on ws://localhost:{WS_PORT}...")
    async with websockets.serve(telemetery_relay, "localhost", WS_PORT):
        await asyncio.Future() # Run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nRelay stopped.")
