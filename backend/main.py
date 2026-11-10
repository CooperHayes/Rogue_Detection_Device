# Import
from fastapi import FastAPI
from pydantic import BaseModel
from database import Base, engine, SessionLocal
from models import ScanRecord


app = FastAPI()


class DiscoveredDevice(BaseModel):
    ip : str
    mac : str

class Scan(BaseModel):
    devices: list[DiscoveredDevice]


Base.metadata.create_all(bind=engine)


@app.post("/scan")
def receive_scan(scan : Scan):
    for device in scan.devices:
        print(f"IP: {device.ip}, MAC: {device.mac}")

    count = len(scan.devices)

    with SessionLocal() as db:
        new_scan = ScanRecord(device_count=count)

        db.add(new_scan)
        db.commit()
        db.refresh(new_scan)

        scan_id = new_scan.id

    return {
        "message": "Scan received successfully",
        "scan_id": scan_id,
        "device_count": len(scan.devices)
    }
    






