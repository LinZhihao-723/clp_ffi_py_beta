from datetime import datetime, tzinfo
import dateutil.tz

def get_formatted_timestamp(timestamp: int, timezone: tzinfo) -> str:
    dt: datetime = datetime.fromtimestamp(timestamp / 1000, timezone)
    return dt.isoformat(sep=" ", timespec="milliseconds")
