import sys
import re

from sqlalchemy import select
from database import SessionLocal
from models import Device, KnownDevice

# Function to manually approve a known device
def approve_device(mac_address, device_name):

    mac = mac_address.strip().upper()

    if not re.fullmatch(
        r"(?:[0-9A-F]{2}:){5}[0-9A-F]{2}", mac
    ):
        print("Invalid MAC address format.")
        return

    with SessionLocal() as db:

        device = db.scalar(
            select(Device).where(
                Device.mac_address == mac
            )
        )

        if device is None:
            print("Device not found in database.")
            return

        existing_approval = db.scalar(
            select(KnownDevice).where(
                KnownDevice.device_id == device.id
            )
        )

        if existing_approval is not None:
            print("Device is already approved.")
            return

        approved_device = KnownDevice(
            device_id=device.id,
            device_name=device_name
        )

        db.add(approved_device)
        db.commit()

        print(f"Approved device: {device_name}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(
            'Usage: python approve_device.py '
            '"MAC_ADDRESS" "DEVICE_NAME"'
        )
        sys.exit(1)

    approve_device(sys.argv[1], sys.argv[2])