from datetime import datetime, tzinfo
import dateutil.tz


def get_formatted_timestamp(timestamp: int, timezone: tzinfo) -> str:
    dt: datetime = datetime.fromtimestamp(timestamp / 1000, timezone)
    return dt.isoformat(sep=" ", timespec="milliseconds")


def get_timezone_from_timezone_id(timezone_id: str) -> tzinfo:
    timezone: tzinfo = dateutil.tz.gettz(timezone_id)
    if timezone is None:
        raise RuntimeError(f"Invalid timezone id: {timezone_id}")
    return timezone
