import dateutil.tz
import pickle
import unittest

from clp_ffi_py.CLPIRDecoder import Message, Metadata
from datetime import tzinfo
from typing import Optional


class TestCaseBase(unittest.TestCase):
    """
    Functionally abstract base class for testing handlers, etc.

    Functionally abstract as we use `load_tests` to avoid adding `TestBase`
    itself to the test suite. However, we cannot mark it as abstract as
    `unittest` will still `__init__` an instance before `load_tests` is run (and
    will error out if any method is marked abstract).
    """


class TestCaseMetadata(TestCaseBase):
    """
    Class for testing clp_ffi_py.CLPIRDecoder.Metadata
    """

    def check_metadata(
        self,
        metadata: Metadata,
        expected_ref_timestamp: int,
        expected_timestamp_format: str,
        expected_timezone_id: str,
    ) -> None:
        """
        Given a Metadata object, check if the content matches the reference
        :param metadata: Metadata object to be checked
        :param expected_ref_timestamp
        :param expected_timestamp_format
        :param expected_timezone_id
        """
        ref_timestamp: int = metadata.get_ref_timestamp()
        timestamp_format: str = metadata.get_timestamp_format()
        timezone_id: str = metadata.get_timezone_id()
        self.assertEqual(
            ref_timestamp,
            expected_ref_timestamp,
            f'Reference Timestamp: "{ref_timestamp}", Expected: "{expected_ref_timestamp}"',
        )
        self.assertEqual(
            timestamp_format,
            expected_timestamp_format,
            f'Timestamp Format: "{timestamp_format}", Expected: "{expected_timestamp_format}"',
        )
        self.assertEqual(
            timezone_id,
            expected_timezone_id,
            f'Timezone ID: "{timezone_id}", Expected: "{expected_timezone_id}"',
        )

        expected_tzinfo: Optional[tzinfo] = dateutil.tz.gettz(expected_timezone_id)
        assert expected_tzinfo is not None
        is_the_same_tz: bool = expected_tzinfo is metadata.timezone
        self.assertEqual(
            is_the_same_tz,
            True,
            f"Timezone does not match timezone id. Timezone ID: {timezone_id}, Timezone:"
            f' {str(metadata.timezone)}"',
        )

    def test_init(self) -> None:
        """
        Test the initialization of Message object without using keyword
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self.check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_keyword_init(self) -> None:
        """
        Test the initialization of Message object using keyword
        """
        ref_timestamp: int = 2005689603270
        timestamp_format: str = "MM/dd/yy HH:mm:ss"
        timezone_id: str = "America/New_York"
        metadata: Metadata

        metadata = Metadata(
            ref_timestamp=ref_timestamp, timestamp_format=timestamp_format, timezone_id=timezone_id
        )
        self.check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        metadata = Metadata(
            timestamp_format=timestamp_format, ref_timestamp=ref_timestamp, timezone_id=timezone_id
        )
        self.check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_timezone_readonly(self) -> None:
        """
        Test the timezone is properly set as readonly and will trigger exception
        for any modification
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self.check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        exception_captured: bool
        wrong_tz: Optional[tzinfo]

        exception_captured = False
        wrong_tz = dateutil.tz.gettz("America/New_York")
        try:
            assert wrong_tz is not None
            metadata.timezone = wrong_tz
        except AttributeError:
            exception_captured = True
        self.assertEqual(
            exception_captured, True, "Timezone is overwritten by another valid tzinfo object"
        )

        exception_captured = False
        wrong_tz = metadata.timezone
        try:
            assert wrong_tz is not None
            metadata.timezone = wrong_tz
        except AttributeError:
            exception_captured = True
        self.assertEqual(exception_captured, True, "Timezone is overwritten by itself")

        exception_captured = False
        try:
            metadata.timezone = None  # type: ignore
        except AttributeError:
            exception_captured = True
        self.assertEqual(exception_captured, True, "Timezone is overwritten by None")


class TestCaseMessage(TestCaseBase):
    """
    Class for testing clp_ffi_py.CLPIRDecoder.Message
    """

    def check_message(
        self,
        msg: Message,
        expected_log_message: str,
        expected_timestamp: int,
        expected_msg_idx: int,
    ) -> None:
        """
        Given a Message object, check if the content matches the reference
        :param msg: Message object to be checked
        :param expected_log_message
        :param expected_timestamp
        :param expected_msg_idx
        """
        log_message: str = msg.get_message()
        timestamp: int = msg.get_timestamp()
        msg_idx: int = msg.get_message_idx()
        self.assertEqual(
            log_message,
            expected_log_message,
            f'Log message: "{log_message}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(
            timestamp,
            expected_timestamp,
            f'Timestamp: "{timestamp}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(
            msg_idx, expected_msg_idx, f"Message idx: {msg_idx}, Expected: {expected_msg_idx}"
        )

    def test_init(self) -> None:
        """
        Test the initialization of Message object without using keyword
        """
        log_message: str = " This is a test log message"
        timestamp: int = 2005689603190
        msg_idx: int = 3270
        metadata: Optional[Metadata] = None

        msg: Message

        msg = Message(log_message, timestamp, msg_idx, metadata)
        self.check_message(msg, log_message, timestamp, msg_idx)

        msg = Message(log_message, timestamp)
        self.check_message(msg, log_message, timestamp, 0)

    def test_keyword_init(self) -> None:
        """
        Test the initialization of Message object using keyword
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        msg_idx: int = 14111813
        metadata: Optional[Metadata] = None

        # Initialize with keyword (in-order)
        msg = Message(
            message=log_message, timestamp=timestamp, message_idx=msg_idx, metadata=metadata
        )
        self.check_message(msg, log_message, timestamp, msg_idx)

        # Initialize with keyword (out-of-order)
        msg = Message(
            message_idx=msg_idx, timestamp=timestamp, message=log_message, metadata=metadata
        )
        self.check_message(msg, log_message, timestamp, msg_idx)

        # Initialize with keyword and default argument (out-of-order)
        msg = Message(timestamp=timestamp, message=log_message)
        self.check_message(msg, log_message, timestamp, 0)

    def test_raw_message(self) -> None:
        """
        Test the reconstruction of the raw message. In particular, it checks if
        the timestamp is properly formatted with the expected tzinfo
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        msg_idx: int = 3190
        metadata: Optional[Metadata] = Metadata(0, "yy/MM/dd HH:mm:ss", "Asia/Hong_Kong")
        msg: Message
        expected_raw_message: str
        raw_message: str

        # If metadata is given, use the metadata's timezone as default
        msg = Message(
            message=log_message, timestamp=timestamp, message_idx=msg_idx, metadata=metadata
        )
        self.check_message(msg, log_message, timestamp, msg_idx)
        expected_raw_message = f"1999-07-23 18:00:00.000+08:00{log_message}"
        raw_message = msg.get_raw_message()

        self.assertEqual(
            raw_message,
            expected_raw_message,
            f"Raw message: {raw_message}; Expected: {expected_raw_message}",
        )

        # If metadata is given but another timestamp is specified, use the given
        # timestamp
        test_tz: Optional[tzinfo] = dateutil.tz.gettz("America/New_York")
        assert test_tz is not None
        expected_raw_message = f"1999-07-23 06:00:00.000-04:00{log_message}"
        raw_message = msg.get_raw_message(test_tz)
        self.assertEqual(
            raw_message,
            expected_raw_message,
            f"Raw message: {raw_message}; Expected: {expected_raw_message}",
        )

        # If the metadata is initialized as None, and no tzinfo passed in, UTC
        # will be used as default
        msg = Message(message=log_message, timestamp=timestamp, message_idx=msg_idx, metadata=None)
        self.check_message(msg, log_message, timestamp, msg_idx)
        expected_raw_message = f"1999-07-23 10:00:00.000+00:00{log_message}"
        raw_message = msg.get_raw_message()
        self.assertEqual(
            raw_message,
            expected_raw_message,
            f"Raw message: {raw_message}; Expected: {expected_raw_message}",
        )

    def test_pickle(self) -> None:
        """
        Test the reconstruction of Message object from pickling data.
        For unpickled Message object, even though the metadata is set to None,
        it should still format the timestamp with the original tz before
        pickling
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        msg_idx: int = 3190
        metadata: Optional[Metadata] = Metadata(0, "yy/MM/dd HH:mm:ss", "Asia/Hong_Kong")
        msg = Message(
            message=log_message, timestamp=timestamp, message_idx=msg_idx, metadata=metadata
        )
        self.check_message(msg, log_message, timestamp, msg_idx)
        reconstructed_msg: Message = pickle.loads(pickle.dumps(msg))
        self.check_message(reconstructed_msg, log_message, timestamp, msg_idx)

        # For unpickled Message object, even though the metadata is set to None,
        # it should still format the timestamp with the original tz before
        # pickling
        expected_raw_message = f"1999-07-23 18:00:00.000+08:00{log_message}"
        raw_message = reconstructed_msg.get_raw_message()
        self.assertEqual(
            raw_message,
            expected_raw_message,
            f"Raw message: {raw_message}; Expected: {expected_raw_message}",
        )


if __name__ == "__main__":
    unittest.main()
