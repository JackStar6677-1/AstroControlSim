import socket
import struct
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import math

# Protocol
# < = Little Endian (orden de bytes estándar en PC)
# B = Unsigned Char (1 byte) -> Packet Type
# I = Unsigned Int (4 bytes) -> ID
# d = Double (8 bytes) -> Azimuth
# d = Double (8 bytes) -> Elevation
# B = Unsigned Char (1 byte) -> State
PACKET_FMT = '<BIddB' 
PACKET_SIZE = struct.calcsize(PACKET_FMT)

CMD_MOVE = 1
TELEMETRY = 2
CMD_GET_TELEMETRY = 3

class Dashboard:
    def __init__(self):
        self.antennas = {} # Diccionario: ID -> (Az, El)
        # Configuración del Gráfico (Matplotlib)
        self.fig, self.ax = plt.subplots()
        self.scat = self.ax.scatter([], [], c=[], cmap='viridis', s=100)
        self.ax.set_xlim(0, 360) # Azimut: 0 a 360 grados
        self.ax.set_ylim(0, 90)  # Elevación: 0 a 90 grados
        self.ax.set_xlabel('Azimut (deg)')
        self.ax.set_ylabel('Elevación (deg)')
        self.ax.set_title('ALMA Array Status - Telemetría en Vivo')
        self.ax.grid(True)
        self.sock = None

    def connect(self):
        try:
            # Crear Socket TCP (El "Teléfono" para llamar al servidor C++)
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(('127.0.0.1', 9000)) # IP Local, Puerto 9000
            print("Conectado al Servidor de Antenas!")
        except Exception as e:
            print(f"Fallo de conexión: {e}")
            self.sock = None

    def request_telemetry(self):
        if not self.sock: return
        try:
            # Enviar Solicitud (Byte Packet)
            # struct.pack convierte números de Python a bytes binarios que C++ entiende
            req = struct.pack(PACKET_FMT, CMD_GET_TELEMETRY, 0, 0.0, 0.0, 0)
            self.sock.sendall(req)
            
            # Read Responses (Expect 50 packets)
            # Warning: Blocking read. In production use async.
            for _ in range(50): 
                data = self.sock.recv(PACKET_SIZE)
                if len(data) < PACKET_SIZE: break
                
                p_type, p_id, p_az, p_el, p_state = struct.unpack(PACKET_FMT, data)
                if p_type == TELEMETRY:
                    self.antennas[p_id] = (p_az, p_el)
                    
        except Exception as e:
            print(f"Error requesting telemetry: {e}")
            self.sock = None # Force reconnect logic if needed

    def update(self, frame):
        self.request_telemetry()
        
        # Prepare data for plot
        x = []
        y = []
        colors = []
        total_signal = 0
        active_antennas = 0
        
        # Image dimensions for sampling
        img_h, img_w = 0, 0
        if 'image_data' in globals():
             img_h, img_w = image_data.shape
        
        for ant_id, (az, el) in self.antennas.items():
            x.append(az)
            y.append(el)
            colors.append(el) 
            
            # SIGNAL SIMULATION (Sampling Real Data)
            if 'image_data' in globals():
                # Map Az/El to Pixel Coords
                px = int((az / 360.0) * img_w)
                py = int((el / 90.0) * img_h)
                
                # Check bounds
                if 0 <= px < img_w and 0 <= py < img_h:
                    signal = image_data[py, px]
                    total_signal += float(signal)
                    active_antennas += 1

        self.scat.set_offsets(list(zip(x, y)))
        self.scat.set_array(colors)
        
        # Update Title with Signal Info
        if active_antennas > 0:
            avg_sig = total_signal / active_antennas
            self.ax.set_title(f'ALMA Status - Signal Strength: {avg_sig:.2f} (Real Data)')
            
        return self.scat,

    def start(self):
        self.connect()
        # Update every 200ms
        self.ani = animation.FuncAnimation(self.fig, self.update, interval=200, blit=False)
        plt.show()

if __name__ == "__main__":
    # Attempt to load FITS data for background
    try:
        from astropy.io import fits
        from astropy.visualization import astropy_mpl_style
        plt.style.use(astropy_mpl_style)
        
        print("Loading FITS data...")
        hdul = fits.open('data/target.fits')
        image_data = hdul[0].data
        hdul.close()
        
        dash = Dashboard()
        
        # Display Image (Map 0-360 Az and 0-90 El to image pixels)
        # For simulation, we just stretch the image to cover the sky plot
        dash.ax.imshow(image_data, cmap='inferno', origin='lower', extent=[0, 360, 0, 90], alpha=0.8)
        print("Sky Background Loaded.")
        
    except Exception as e:
        print(f"Could not load FITS data: {e}")
        print("Running in stand-alone mode (Black Sky).")
        dash = Dashboard()

    dash.start()
