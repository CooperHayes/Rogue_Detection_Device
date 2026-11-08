# Import
from fastapi import FastAPI
from pydantic import BaseModel


app = FastAPI()


class DiscoveredDevice(BaseModel):
    ip : str
    mac : str

class Scan(BaseModel):
    devices: list[DiscoveredDevice]


@app.post("/scan")
def receive_scan(scan : Scan):
    for device in scan.devices:
        print(f"IP: {device.ip}, MAC: {device.mac}")

    print(f"Device count: {len(scan.devices)}")

    return {
        "message": "Scan received successfully",
        "device_count": len(scan.devices)
    }
    






