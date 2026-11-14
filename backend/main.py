# Import
from fastapi import FastAPI
from pydantic import BaseModel
from database import Base, engine, SessionLocal
from models import ScanRecord, Device, ScanDevice
from sqlalchemy import select
from sqlalchemy import ForeignKey, UniqueConstraint


app = FastAPI()


class DiscoveredDevice(BaseModel):
    ip : str
    mac : str

class Scan(BaseModel):
    devices: list[DiscoveredDevice]


Base.metadata.create_all(bind=engine)


@app.post("/scan")
def receive_scan(scan : Scan):
    # Record the number of devices received from the ESP32 discovery scan
    count = len(scan.devices)

    with SessionLocal() as db:
        # Handle exceptions
        try:
            # Stage an entry of a scan to the SQLite database
            new_scan = ScanRecord(device_count=count)
            db.add(new_scan)

            # Iterate through the devices in the JSON
            for discovered in scan.devices:
                
                # Parse the MAC address of each device
                mac = discovered.mac.strip().upper()

                # Determine if the device is already in the database
                existing_device = db.scalar(
                    select(Device).where(
                        Device.mac_address == mac
                    )
                )

                # If there is a device with the MAC address existing skip adding a new device
                if existing_device is None:
                    new_device = Device(mac_address=mac)
                    db.add(new_device)

                    print(f"New device: {mac}")
                else:
                    print(f"Existing device: {mac}")
            
            # Commit the changes staged by db.add()
            db.commit()
            db.refresh(new_scan)

            scan_id = new_scan.id

        except Exception:
            db.rollback()
            raise

    # Return a JSON with the scan ID and number of devices received
    return {
        "message": "Scan received successfully",
        "scan_id": scan_id,
        "device_count": len(scan.devices)
    }
    






