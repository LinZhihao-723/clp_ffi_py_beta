from pathlib import Path
from sys import stderr
from types import TracebackType
from typing import Generator, IO, Iterator, Optional, Type, Union

from zstandard import ZstdDecompressionReader, ZstdDecompressor

from clp_ffi_py import Decoder, DecoderBuffer, LogEvent, Metadata, Query


class CLPIRStreamReader:
    """
    This class represents a stream reader used to read/decode encoded log events
    from a CLP IR stream. It also provides method(s) to instantiate a log event
    generator with a customized search query.

    :param istream: Input stream that contains encoded CLP IR.
    :param decoder_buffer_size: Initial size of the decoder buffer.
    :param enable_compression: A flag indicating whether the istream is
    compressed using `zstd`.
    """

    DEFAULT_DECODER_BUFFER_SIZE: int = 65536

    def __init__(
        self,
        istream: IO[bytes],
        decoder_buffer_size: int = DEFAULT_DECODER_BUFFER_SIZE,
        enable_compression: bool = True,
    ):
        self.__istream: Union[IO[bytes], ZstdDecompressionReader]
        if enable_compression:
            dctx: ZstdDecompressor = ZstdDecompressor()
            self.__istream = dctx.stream_reader(istream, read_across_frames=True)
        else:
            self.__istream = istream
        self._decoder_buffer: DecoderBuffer = DecoderBuffer(self.__istream, decoder_buffer_size)
        self._metadata: Optional[Metadata] = None

    def read_next_log_event(self) -> Optional[LogEvent]:
        """
        Reads and decodes the next encoded log event from the IR stream.

        :return: Next unread log event represented as an instance of LogEvent.
        :return: None if the end of IR stream is reached.
        :raise Exception: If `Decoder.decode_next_log_event` fails.
        """
        return Decoder.decode_next_log_event(self._decoder_buffer)

    def read_preamble(self) -> None:
        """
        Try to decode the preamble and set `metadata`. If `metadata` has been
        set already, it will instantly return. It is separated from `__init__`
        so that the input stream does not need to be readable on a reader's
        construction, but until the user starts to iterate logs.

        :raise Exception: If `Decoder.decode_preamble` fails.
        """
        if self.has_metadata():
            return
        self._metadata = Decoder.decode_preamble(self._decoder_buffer)

    def get_metadata(self) -> Metadata:
        if None is self._metadata:
            raise RuntimeError("The metadata has not been successfully decoded yet.")
        return self._metadata

    def has_metadata(self) -> bool:
        return None is not self._metadata

    def search(self, query: Query) -> Generator[LogEvent, None, None]:
        """
        Searches and yields log events that match a specific search query.

        :param query: The input query object used to match log events. Check the
        document of `clp_ffi_py.Query` for more details.
        :yield: The next unread encoded log event that matches the given search
        query from the IR stream.
        """
        if False is self.has_metadata():
            self.read_preamble()
        while True:
            log_event: Optional[LogEvent] = Decoder.decode_next_log_event(
                self._decoder_buffer, query
            )
            if None is log_event:
                break
            yield log_event

    def close(self) -> None:
        self.__istream.close()

    def __iter__(self) -> Iterator[LogEvent]:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    def __enter__(self) -> Iterator[LogEvent]:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    def __next__(self) -> LogEvent:
        next_log_event: Optional[LogEvent] = self.read_next_log_event()
        if None is next_log_event:
            raise StopIteration
        return next_log_event

    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_value: Optional[BaseException],
        traceback: Optional[TracebackType],
    ) -> None:
        self.close()


class CLPIRFileReader(CLPIRStreamReader):
    """
    Wrapper class of `CLPIRStreamReader` that calls `open` for convenience.
    """

    def __init__(
        self,
        fpath: Path,
        decoder_buffer_size: int = CLPIRStreamReader.DEFAULT_DECODER_BUFFER_SIZE,
        enable_compression: bool = True,
    ):
        self._path: Path = fpath
        super().__init__(open(fpath, "rb"), decoder_buffer_size, enable_compression)

    def dump(self, ostream: IO[str] = stderr) -> None:
        for log_event in self:
            ostream.write(str(log_event))
