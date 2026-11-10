from datetime import datetime

from sqlalchemy import DateTime, Integer, func
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