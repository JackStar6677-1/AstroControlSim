# Jackstar Observatory: Principles of Interferometry

Radio interferometry is the art of combining signals from multiple antennas to create a "Virtual Telescope" with the resolution of a dish as large as the maximum distance between antennas (the baseline).

## Core Concepts

### 1. Geometric Phase Delay
When light from a distant star hits an array of antennas, it doesn't arrive at all of them at exactly the same time. Depending on the antenna's position $(x, y)$ and the source direction $(\text{Az}, \text{El})$, there is a path length difference.

In the **Jackstar Simulation**, we calculate this as:
$$\Phi = k \cdot (\vec{B} \cdot \vec{s})$$
where:
- $k$ is the wave number ($2\pi / \lambda$).
- $\vec{B}$ is the baseline vector (antenna position).
- $\vec{s}$ is the unit vector pointing toward the source.

### 2. The Complex Signal
Each antenna in our simulation now generates a complex signal:
- **Amplitude**: Represents the intensity of the incoming light.
- **Phase**: Represents the relative delay of the wave front.

### 3. Correlation (The "Multiplier")
The magic happens in the **Jackstar Correlator**. By cross-multiplying the complex signals from two antennas ($V_1 \times V_2^*$), we extract the "Visibility".
- If the phases are aligned, we get **Constructive Interference**.
- If they are opposed, we get **Destructive Interference**.

## Jackstar Implementation Details
- **Wavelength ($\lambda$)**: Simulated at 1mm (matching ALMA's high-frequency bands).
- **Baselines**: The positions are distributed in a 7x7 grid with 15m spacing.
- **Dynamic Phase**: As the antennas slew or track, the phase shifts automatically, simulating real-time celestial motion.

---
*Created for the Jackstar Observatory project.*
