from clp_ffi_py.CLPIRDecoder import (
    DecoderBuffer,
    decode_preamble,
    decode_next_message_with_query,
    Metadata,
    Message,
    Query,
)
from types import TracebackType
from typing import IO, Iterator, Optional, Type
from pathlib import Path

from datetime import datetime, tzinfo
import dateutil.tz
from zstandard import ZstdDecompressor, ZstdDecompressionReader

class CLPStreamReader:
    def __init__(
        self,
        istream: IO[bytes],
        query: Optional[Query],
        timestamp_format: Optional[str] = None,
    ):
        self.dctx: ZstdDecompressor = ZstdDecompressor()
        self.zstream: ZstdDecompressionReader = self.dctx.stream_reader(
            istream, read_across_frames=True
        )
        self.buffer: DecoderBuffer = DecoderBuffer()
        self.timestamp_format: Optional[str] = timestamp_format
        self.timezone: Optional[tzinfo] = None
        self.metadata: Optional[Metadata] = None
        self.message: Optional[Message] = None
        self.ref_timestamp: int = 0
        self.query = query

    def close(self) -> None:
        self.zstream.close()

    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_value: Optional[BaseException],
        traceback: Optional[TracebackType],
    ) -> None:
        self.close()

    def __iter__(self) -> Iterator[Message]:
        self.metadata = decode_preamble(self.zstream, self.buffer)
        self.ref_timestamp = self.metadata.get_ref_timestamp()
        self.timezone = dateutil.tz.gettz(self.metadata.get_timezone())
        return self

    def __enter__(self) -> Iterator[Message]:
        return self

    def __next__(self) -> Message:
        self.message = decode_next_message_with_query(
            self.ref_timestamp, self.zstream, self.buffer, self.query
        )
        if self.message is None:
            raise StopIteration
        self.ref_timestamp = self.message.get_timestamp()
        return self.message

    def get_raw_message(self, message: Message) -> str:
        dt: datetime = datetime.fromtimestamp(message.get_timestamp() / 1000, self.timezone)
        time_format: str = dt.isoformat(sep=" ", timespec="milliseconds")
        return f"{time_format}{message.get_message()}"

    def decompress(self, fpath: Path):
        self.metadata = decode_preamble(self.zstream, self.buffer)
        self.ref_timestamp = self.metadata.get_ref_timestamp()
        self.timezone = dateutil.tz.gettz(self.metadata.get_timezone())
        with open(fpath, "w") as file_out:
            while True:
                self.message = decode_next_message_with_query(
                    self.ref_timestamp, self.zstream, self.buffer, self.query
                )
                if self.message is None:
                    break
                self.ref_timestamp = self.message.get_timestamp()
                file_out.write(self.get_raw_message(self.message))
        self.close()

class CLPFileReader(CLPStreamReader):
    def __init__(
        self,
        fpath: Path,
        query: Optional[Query],
        timestamp_format: Optional[str] = None,
    ):
        self.path: Path = fpath
        super().__init__(open(fpath, "rb"), query, timestamp_format)
