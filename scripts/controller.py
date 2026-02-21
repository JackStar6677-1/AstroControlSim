import socket
import struct
import time
import math

# Protocol
PACKET_FMT = '<BIddB' # type, id, az, el, state
CMD_MOVE = 1

def send_move(sock, ant_id, az, el):
    packet = struct.pack(PACKET_FMT, CMD_MOVE, ant_id, az, el, 0)
    sock.sendall(packet)

def main():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', 9000))
        print("Connected to AstroControlSim Controller Port")
    except Exception as e:
        print(f"Could not connect: {e}")
        return

    print("Starting 'Calibration Config' (Scanning Pattern)...")
    
    # Move all antennas in a wave pattern
    try:
        t = 0
        while True:
            for i in range(1, 51):
                # Create a wave effect based on ID and time
                az = (t * 10 + i * 5) % 360
                el = 45 + 30 * math.sin(t * 0.5 + i * 0.1)
                
                send_move(sock, i, az, el)
            
            print(f"Sent wave update t={t}")
            t += 0.5
            time.sleep(0.5)
            
    except KeyboardInterrupt:
        print("Stopping controller...")
        sock.close()

if __name__ == "__main__":
    main()
