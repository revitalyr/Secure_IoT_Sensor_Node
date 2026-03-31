#!/usr/bin/env python3
"""
Secure IoT Sensor Node OTA Firmware Uploader
Production-ready firmware update tool with CRC verification and progress tracking
"""

import serial
import sys
import time
import hashlib
import struct
import argparse
from pathlib import Path

class OTAUploader:
    def __init__(self, port, baudrate=115200, timeout=10):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial = None
        
        # OTA protocol constants
        self.CMD_START_OTA = 0x01
        self.CMD_SEND_CHUNK = 0x02
        self.CMD_VERIFY_CRC = 0x03
        self.CMD_COMMIT = 0x04
        self.CMD_GET_STATUS = 0x05
        self.RESP_OK = 0x00
        self.RESP_ERROR = 0xFF
        self.CHUNK_SIZE = 256
        
    def connect(self):
        """Establish serial connection"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            print(f"✓ Connected to {self.port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"✗ Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Close serial connection"""
        if self.serial:
            self.serial.close()
            print("✓ Disconnected")
    
    def send_command(self, cmd, data=None):
        """Send command with protocol framing"""
        packet = struct.pack('<BH', cmd, len(data) if data else 0)
        if data:
            packet += data
        
        self.serial.write(packet)
        self.serial.flush()
        
        # Read response
        resp = self.serial.read(1)
        return resp[0] if resp else None
    
    def get_device_status(self):
        """Get current device status"""
        status = self.send_command(self.CMD_GET_STATUS)
        if status == self.RESP_OK:
            print("✓ Device ready for OTA")
            return True
        else:
            print(f"✗ Device not ready (status: {status:#02x})")
            return False
    
    def start_ota(self, firmware_size, firmware_crc):
        """Initialize OTA on device"""
        data = struct.pack('<II', firmware_size, firmware_crc)
        resp = self.send_command(self.CMD_START_OTA, data)
        
        if resp == self.RESP_OK:
            print(f"✓ OTA started for {firmware_size} bytes")
            return True
        else:
            print("✗ Failed to start OTA")
            return False
    
    def send_firmware_chunk(self, chunk_data, chunk_num, total_chunks):
        """Send firmware chunk with progress"""
        # Add chunk header
        header = struct.pack('<I', chunk_num)
        packet = header + chunk_data
        
        resp = self.send_command(self.CMD_SEND_CHUNK, packet)
        
        # Progress bar
        progress = (chunk_num + 1) / total_chunks
        bar_length = 50
        filled = int(bar_length * progress)
        bar = '█' * filled + '░' * (bar_length - filled)
        print(f'\rProgress: |{bar}| {progress*100:.1f}% ({chunk_num+1}/{total_chunks})', end='', flush=True)
        
        return resp == self.RESP_OK
    
    def verify_firmware_crc(self, expected_crc):
        """Verify firmware CRC on device"""
        resp = self.send_command(self.CMD_VERIFY_CRC)
        if resp == self.RESP_OK:
            print("\n✓ Firmware CRC verified")
            return True
        else:
            print("\n✗ Firmware CRC verification failed")
            return False
    
    def commit_firmware(self):
        """Commit firmware (make it active)"""
        print("Committing firmware...")
        resp = self.send_command(self.CMD_COMMIT)
        
        if resp == self.RESP_OK:
            print("✓ Firmware committed successfully")
            return True
        else:
            print("✗ Failed to commit firmware")
            return False
    
    def calculate_firmware_crc(self, firmware_data):
        """Calculate CRC32 of firmware"""
        crc = 0xFFFFFFFF
        for byte in firmware_data:
            crc ^= byte
            for _ in range(8):
                if crc & 1:
                    crc = (crc >> 1) ^ 0xEDB88320
                else:
                    crc >>= 1
        return crc ^ 0xFFFFFFFF
    
    def upload_firmware(self, firmware_path):
        """Main OTA upload process"""
        print(f"🚀 Starting OTA upload: {firmware_path}")
        
        # Load firmware
        try:
            with open(firmware_path, 'rb') as f:
                firmware_data = f.read()
        except FileNotFoundError:
            print(f"✗ Firmware file not found: {firmware_path}")
            return False
        
        firmware_size = len(firmware_data)
        firmware_crc = self.calculate_firmware_crc(firmware_data)
        total_chunks = (firmware_size + self.CHUNK_SIZE - 1) // self.CHUNK_SIZE
        
        print(f"Firmware: {firmware_size} bytes, CRC: {firmware_crc:08X}")
        
        # Connect to device
        if not self.connect():
            return False
        
        try:
            # Check device status
            if not self.get_device_status():
                return False
            
            # Start OTA
            if not self.start_ota(firmware_size, firmware_crc):
                return False
            
            # Send firmware chunks
            for i in range(0, firmware_size, self.CHUNK_SIZE):
                chunk = firmware_data[i:i + self.CHUNK_SIZE]
                chunk_num = i // self.CHUNK_SIZE
                
                if not self.send_firmware_chunk(chunk, chunk_num, total_chunks):
                    print(f"\n✗ Failed to send chunk {chunk_num}")
                    return False
                
                # Small delay to prevent overwhelming the device
                time.sleep(0.01)
            
            # Verify firmware
            if not self.verify_firmware_crc(firmware_crc):
                return False
            
            # Commit firmware
            if not self.commit_firmware():
                return False
            
            print("\n🎉 OTA upload completed successfully!")
            print("Device will reboot with new firmware...")
            return True
            
        except Exception as e:
            print(f"\n✗ OTA upload failed: {e}")
            return False
        
        finally:
            self.disconnect()

def main():
    parser = argparse.ArgumentParser(description='Secure IoT OTA Firmware Uploader')
    parser.add_argument('--device', required=True, help='Serial port (e.g., /dev/ttyUSB0 or COM3)')
    parser.add_argument('--firmware', required=True, help='Firmware binary file')
    parser.add_argument('--baudrate', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--timeout', type=int, default=10, help='Timeout in seconds (default: 10)')
    
    args = parser.parse_args()
    
    # Validate firmware file
    firmware_path = Path(args.firmware)
    if not firmware_path.exists():
        print(f"✗ Firmware file not found: {args.firmware}")
        sys.exit(1)
    
    # Create uploader and start OTA
    uploader = OTAUploader(args.device, args.baudrate, args.timeout)
    success = uploader.upload_firmware(str(firmware_path))
    
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
