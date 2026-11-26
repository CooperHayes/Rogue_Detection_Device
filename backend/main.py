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

class DeviceResponse(BaseModel):
    id: int
    mac_address: str
    approved : bool


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
                    
                    new_alert = Alert(
                        device_id=device.id,
                        scan_id=scan_id,
                        alert_type="UNAPPROVED_DEVICE"
                        )

                    db.add(new_alert)

                    print(f"New alert registered: {mac}")

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



@app.get("/devices" , response_model = list[DeviceResponse])
def get_devices():
    
    with SessionLocal() as db:

        # Retrieve devices and matching approval records
        statement = (
            select(Device, KnownDevice)
            .outerjoin(
                KnownDevice,
                Device.id == KnownDevice.device_id
            )
            .order_by(Device.id)
        )

        # Execute the query
        results = db.execute(statement).all()

        # Construct the response
        return [
            DeviceResponse(
                id=device.id,
                mac_address=device.mac_address,
                approved=approval is not None
            )
            for device, approval in results
        ]
    






