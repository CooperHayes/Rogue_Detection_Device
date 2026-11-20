# Import
from fastapi import FastAPI
from pydantic import BaseModel
from database import Base, engine, SessionLocal
from models import ScanRecord, Device, ScanDevice, KnownDevice, Alert
from sqlalchemy import select



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
            db.flush()
            scan_id = new_scan.id

            # Iterate through the devices in the JSON
            for discovered in scan.devices:
                
                # Parse the MAC address of each device
                mac = discovered.mac.strip().upper()

                # Determine if the device is already in the database
                device = db.scalar(
                    select(Device).where(
                        Device.mac_address == mac
                    )
                )

                # If there is a device with the MAC address existing skip adding a new device
                if device is None:
                    device = Device(mac_address=mac)
                    db.add(device)
                    db.flush()

                    print(f"New device: {mac}")
                else:
                    print(f"Existing device: {mac}")
                
                # Check whether this device is approved
                approval = db.scalar(
                    select(KnownDevice).where(
                    KnownDevice.device_id == device.id
                )
                )

                if approval is None:
                    print(f"UNAPPROVED DEVICE: {mac}")

                    # Search for an existing active alert
                    existing_alert = db.scalar(
                        select(Alert).where(
                            Alert.device_id == device.id,
                            Alert.alert_type == "UNAPPROVED_DEVICE",
                            Alert.resolved_at.is_(None)
                        )
                    )
                    # Create an alert only if one does not exist
                    if existing_alert is None:

                        new_alert = Alert(
                            device_id=device.id,
                            scan_id=scan_id,
                            alert_type="UNAPPROVED_DEVICE"
                        )

                        db.add(new_alert)

                        print(f"New alert registered: {mac}")

                    else:
                        print(f"Active alert already exists: {mac}")
                else:
                    print(f"APPROVED DEVICE: {approval.device_name}")
                
                # Add all observations of devices to scan_devices
                observation = ScanDevice(
                    scan_id=scan_id,
                    device_id=device.id,
                    ip_address=discovered.ip
                )

                db.add(observation)
            
            # Commit the changes staged by db.add()
            db.commit()
            

        except Exception:
            db.rollback()
            raise

    # Return a JSON with the scan ID and number of devices received
    return {
        "message": "Scan received successfully",
        "scan_id": scan_id,
        "device_count": len(scan.devices)
    }
    






