import socket
import struct
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import math

# Protocol
# < = Little Endian
# B = Unsigned Char (1 byte) -> Packet Type
# I = Unsigned Int (4 bytes) -> ID
# d = Double (8 bytes) -> Azimuth
# d = Double (8 bytes) -> Elevation
# d = Double (8 bytes) -> AzError
# d = Double (8 bytes) -> ElError
# d = Double (8 bytes) -> PosX
# d = Double (8 bytes) -> PosY
# d = Double (8 bytes) -> SignalAmp
# d = Double (8 bytes) -> SignalPhase
# d = Double (8 bytes) -> MotorTemp
# d = Double (8 bytes) -> MotorCurrent
# B = Unsigned Char (1 byte) -> State
PACKET_FMT = '<BIdddddddddd B' 
PACKET_SIZE = struct.calcsize(PACKET_FMT)

CMD_MOVE = 1
TELEMETRY = 2
CMD_GET_TELEMETRY = 3

class Dashboard:
    def __init__(self):
        self.antennas = {} # ID -> (Az, El, AzErr, ElErr, X, Y, Amp, Phase)
        # Configuración del Gráfico (Matplotlib)
        self.fig, (self.ax, self.ax_corr) = plt.subplots(1, 2, figsize=(12, 5))
        
        self.scat = self.ax.scatter([], [], c=[], cmap='viridis', s=100)
        self.ax.set_xlim(0, 360) 
        self.ax.set_ylim(0, 90)  
        self.ax.set_title('Jackstar Sky View (PID Control)')
        
        # Correlación Plot
        self.scat_corr = self.ax_corr.scatter([], [], c='cyan', s=50, alpha=0.5)
        self.ax_corr.set_xlim(-100, 100)
        self.ax_corr.set_ylim(-100, 100)
        self.ax_corr.set_title('Interferometry (UV Plane / Baselines)')
        self.ax_corr.set_xlabel('U (meters)')
        self.ax_corr.set_ylabel('V (meters)')
        
        self.sock = None

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(('127.0.0.1', 9000)) 
            print("Conectado al Servidor de Antenas!")
        except Exception as e:
            print(f"Fallo de conexión: {e}")
            self.sock = None

    def request_telemetry(self):
        if not self.sock: return
        try:
            # We send 0s for the 6 extra doubles in the request packet too
            req = struct.pack(PACKET_FMT, CMD_GET_TELEMETRY, 0, 0,0,0,0,0,0,0,0,0,0, 0)
            self.sock.sendall(req)
            
            for _ in range(50): 
                data = self.sock.recv(PACKET_SIZE)
                if len(data) < PACKET_SIZE: break
                
                # Unpack all 13 fields
                p_type, p_id, p_az, p_el, p_ae, p_ee, p_x, p_y, p_sa, p_sp, p_mt, p_mc, p_st = struct.unpack(PACKET_FMT, data)
                if p_type == TELEMETRY:
                    self.antennas[p_id] = (p_az, p_el, p_ae, p_ee, p_x, p_y, p_sa, p_sp)
                    
        except Exception as e:
            print(f"Error requesting telemetry: {e}")
            self.sock = None # Force reconnect logic if needed

    def update(self, frame):
        self.request_telemetry()
        
        x, y, colors = [], [], []
        uv_u, uv_v = [], []
        total_signal = 0
        active_antennas = 0
        total_error = 0.0
        
        # Image dimensions for sampling
        img_h, img_w = 0, 0
        if 'image_data' in globals():
             img_h, img_w = image_data.shape
        
        ant_list = list(self.antennas.values())
        for i, (az, el, azerr, elerr, px, py, amp, ph) in enumerate(ant_list):
            x.append(az)
            y.append(el)
            colors.append(el) 
            total_error += math.sqrt(azerr**2 + elerr**2)
            
            # Simulated UV Plane (Correlation between this antenna and the next)
            if i < len(ant_list) - 1:
                next_ant = ant_list[i+1]
                # Baseline vector (U, V)
                uv_u.append(px - next_ant[4])
                uv_v.append(py - next_ant[5])

            # SIGNAL SIMULATION (Sampling Real Data)
            if 'image_data' in globals():
                idx_x = int((az / 360.0) * img_w)
                idx_y = int((el / 90.0) * img_h)
                if 0 <= idx_x < img_w and 0 <= idx_y < img_h:
                    signal = image_data[idx_y, idx_x]
                    total_signal += float(signal)
                    active_antennas += 1

        self.scat.set_offsets(list(zip(x, y)))
        self.scat.set_array(colors)
        
        if uv_u:
            self.scat_corr.set_offsets(list(zip(uv_u, uv_v)))
        
        if len(self.antennas) > 0:
            avg_err = total_error / len(self.antennas)
            avg_sig = total_signal / active_antennas if active_antennas > 0 else 0
            self.ax.set_title(f'Avg Error: {avg_err:.3f}° | Signal: {avg_sig:.1f}')
            
        return self.scat, self.scat_corr,

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
