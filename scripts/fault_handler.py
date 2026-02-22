import socket
import struct
import time

# Protocol
PACKET_FMT = '<BIddB' # type, id, az, el, state
PACKET_SIZE = struct.calcsize(PACKET_FMT)

CMD_MOVE = 1
TELEMETRY = 2
CMD_GET_TELEMETRY = 3
CMD_RESET = 4

# Antenna States
STATE_FAULT = 3

def main():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', 9000))
        print("Connected to Jackstar Observatory - Operator Module")
    except Exception as e:
        print(f"Connection failed: {e}")
        return

    print("Monitoring Array Health...")
    
    try:
        while True:
            # Request Telemetry
            req = struct.pack(PACKET_FMT, CMD_GET_TELEMETRY, 0, 0.0, 0.0, 0)
            sock.sendall(req)
            
            # Read Responses (Expect 50 packets)
            for _ in range(50): 
                data = sock.recv(PACKET_SIZE)
                if len(data) < PACKET_SIZE: break
                
                _, p_id, p_az, p_el, p_state = struct.unpack(PACKET_FMT, data)
                
                if p_state == STATE_FAULT:
                    print(f"[!] FAULT DETECTED ON ANTENNA {p_id} -> RESETTING...")
                    # Send Reset Packet
                    reset_pkt = struct.pack(PACKET_FMT, CMD_RESET, p_id, 0.0, 0.0, 0)
                    sock.sendall(reset_pkt)
                    
            time.sleep(1) # Monitor every second
            
    except KeyboardInterrupt:
        print("Stopping operator...")
        sock.close()

if __name__ == "__main__":
    main()
