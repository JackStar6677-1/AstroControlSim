import socket
import struct
import time

# Protocol definition
# struct Packet {
#     uint8_t type;      (1 byte)
#     uint32_t antennaId; (4 bytes)
#     double azimuth;    (8 bytes)
#     double elevation;  (8 bytes)
# };
# Total size: 1+4+8+8 = 21 bytes. Warning: C++ struct alignment might add padding!
# In Protocol.h we used #pragma pack(push, 1) so size is exactly 21.

PACKET_FMT = '<BIdd' # Little endian, UByte, UInt, Double, Double
CMD_MOVE = 1

def send_command(antenna_id, az, el):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(('127.0.0.1', 9000))
        print(f"Connected! Sending Move Command to Antenna {antenna_id}: Az={az}, El={el}")
        
        packet = struct.pack(PACKET_FMT, CMD_MOVE, antenna_id, az, el)
        sock.sendall(packet)
        print("Packet sent.")
        
    except ConnectionRefusedError:
        print("Error: Could not connect to AstroControlSim (is it running?)")
    finally:
        sock.close()

if __name__ == "__main__":
    # Test: Move Antenna 1 to Az 180, El 45
    send_command(1, 180.0, 45.0)
    
    time.sleep(1)
    
    # Test: Move Antenna 50 to Az 90, El 30
    send_command(50, 90.0, 30.0)
