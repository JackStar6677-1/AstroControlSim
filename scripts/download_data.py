from astropy.utils.data import download_file
import shutil
import os

def main():
    print("Downloading Real Astronomical Data (FITS)...")
    
    # URL for Horsehead Nebula (Standard Astropy Example)
    # We use this because accurate ALMA FITS files are large and harder to direct-link reliably without a portal account.
    url = 'http://data.astropy.org/tutorials/FITS-images/HorseHead.fits'
    
    try:
        # Download (caches automatically)
        tmp_path = download_file(url, cache=True)
        
        # Move to data/target.fits
        os.makedirs('data', exist_ok=True)
        dest = 'data/target.fits'
        
        shutil.copy(tmp_path, dest)
        print(f"Success! Data saved to: {dest}")
        print("Target: Horsehead Nebula (Simulating ALMA Observation)")
        
    except Exception as e:
        print(f"Error downloading data: {e}")

if __name__ == "__main__":
    main()
