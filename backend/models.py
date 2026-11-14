from datetime import datetime

from sqlalchemy import DateTime, Integer, String, func
from sqlalchemy.orm import Mapped, mapped_column

from database import Base


class ScanRecord(Base):
    __tablename__ = "scans"

    id: Mapped[int] = mapped_column(primary_key=True)

    timestamp: Mapped[datetime] = mapped_column(
        DateTime,
        server_default=func.current_timestamp()
    )

    device_count: Mapped[int] = mapped_column(Integer)

class Device(Base):
    __tablename__ = "devices"

    id: Mapped[int] = mapped_column(primary_key=True)

    mac_address: Mapped[str] = mapped_column(
        String(17),
        unique=True,
        nullable=False
    )