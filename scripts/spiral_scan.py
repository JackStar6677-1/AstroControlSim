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
        print("Connected to AstroControlSim - spiral_scan")
    except Exception as e:
        print(f"Could not connect: {e}")
        return

    print("Executing START_SPIRAL_SEARCH pattern...")
    
    try:
        t = 0
        while True:
            # Archimedean spiral: r = a + b*theta
            # Map this to Az/El
            
            for i in range(1, 51):
                # Each antenna follows the spiral but with a phase offset
                theta = t * 0.5 + (i * 0.1)
                radius = 5 + (2 * theta) % 80 # Spiral out up to 85 deg El
                
                az = (theta * 50) % 360
                el = radius
                if el > 90: el = 90
                
                send_move(sock, i, az, el)
            
            t += 0.1
            time.sleep(0.1) # 10Hz updates
            
    except KeyboardInterrupt:
        print("Stopping spiral scan...")
        sock.close()

if __name__ == "__main__":
    main()
