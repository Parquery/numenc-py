#!/usr/bin/env python3
# pylint: disable=missing-docstring
import unittest

import hypothesis
import hypothesis.strategies
import numenc

MIN_SIGNED_INT64 = -9223372036854775808
MAX_SIGNED_INT64 = 9223372036854775807
MAX_UNSIGNED_INT64 = 18446744073709551615


class TestEncodingInt64(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT64, max_value=MAX_SIGNED_INT64))
    def test_encode_decode_automatic(self, value: int):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_int64(numenc.from_int64(value)))

    @hypothesis.given(
        value1=hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT64, max_value=MAX_SIGNED_INT64),
        value2=hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT64, max_value=MAX_SIGNED_INT64))
    def test_encode_order(self, value1: int, value2: int):
        encoded1 = numenc.from_int64(value1)
        encoded2 = numenc.from_int64(value2)
        if value1 < value2:
            self.assertLess(encoded1, encoded2)
        elif value1 == value2:
            self.assertEqual(encoded1, encoded2)
        else:
            self.assertGreater(encoded1, encoded2)

    def test_encode_decode(self):
        # yapf: disable
        nums = [
            MIN_SIGNED_INT64,
            -3425243,
            -3,
            0,
            3,
            42345235,
            MAX_SIGNED_INT64
        ]

        expected = [
            b'\x00\x00\x00\x00\x00\x00\x00\x00',
            b'\x7f\xff\xff\xff\xff\xcb\xbc\x25',
            b'\x7f\xff\xff\xff\xff\xff\xff\xfd',
            b'\x80\x00\x00\x00\x00\x00\x00\x00',
            b'\x80\x00\x00\x00\x00\x00\x00\x03',
            b'\x80\x00\x00\x00\x02\x86\x23\x13',
            b'\xff\xff\xff\xff\xff\xff\xff\xff'
        ]
        # yapf: enable

        for i, num in enumerate(nums):
            key = numenc.from_int64(num)
            self.assertEqual(expected[i], key)

            num_decoded = numenc.to_int64(key)
            self.assertEqual(num, num_decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLess(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = [
            "some string", 12.3, ('1', '2'), [], {}, b'\x01\x02',
            MIN_SIGNED_INT64 - 1, MAX_SIGNED_INT64 + 1, MAX_UNSIGNED_INT64,
            MAX_UNSIGNED_INT64**2
        ]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_int64(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

    def test_decode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, 232, -1.23]
        val_err_triggers = [
            b'\x01', b'', b'\x01\x02\x02\x02',
            b'\x01\x02\x01\x02\x01\x02\x01\x02\x02\x02'
        ]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.to_int64(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertTrue(
                type_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

        for weird_val in val_err_triggers:
            val_err = None
            try:
                _ = numenc.to_int64(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertTrue(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))


class TestEncodingUInt64(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT64))
    def test_encode_decode_automatic(self, value: int):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_uint64(numenc.from_uint64(value)))

    @hypothesis.given(
        value1=hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT64),
        value2=hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT64))
    def test_encode_order(self, value1: int, value2: int):
        encoded1 = numenc.from_uint64(value1)
        encoded2 = numenc.from_uint64(value2)
        if value1 < value2:
            self.assertLess(encoded1, encoded2)
        elif value1 == value2:
            self.assertEqual(encoded1, encoded2)
        else:
            self.assertGreater(encoded1, encoded2)

    def test_encode_decode(self):
        # yapf: disable
        nums = [
            0,
            9223372036851350565,
            9223372036854775805,
            9223372036854775808,
            9223372036854775811,
            9223372036897121043,
            MAX_UNSIGNED_INT64
        ]

        expected = [
            b'\x00\x00\x00\x00\x00\x00\x00\x00',
            b'\x7f\xff\xff\xff\xff\xcb\xbc\x25',
            b'\x7f\xff\xff\xff\xff\xff\xff\xfd',
            b'\x80\x00\x00\x00\x00\x00\x00\x00',
            b'\x80\x00\x00\x00\x00\x00\x00\x03',
            b'\x80\x00\x00\x00\x02\x86\x23\x13',
            b'\xff\xff\xff\xff\xff\xff\xff\xff'
        ]
        # yapf: enable

        for i, num in enumerate(nums):
            key = numenc.from_uint64(num)
            self.assertEqual(expected[i], key)

            num_decoded = numenc.to_uint64(key)
            self.assertEqual(num, num_decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLess(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = [
            "some string", ('1', '2'), [], {}, b'\x01\x02', -2222222, -1,
            MAX_UNSIGNED_INT64 + 1, MAX_UNSIGNED_INT64**2
        ]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_uint64(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual("Wrong input: expected unsigned 64-bit integer.",
                             str(type_err))

    def test_decode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, 232, -1.23]
        val_err_triggers = [
            b'\x01', b'', b'\x01\x02\x02\x02',
            b'\x01\x02\x01\x02\x01\x02\x01\x02\x02\x02'
        ]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.to_uint64(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual("Wrong input: expected bytes.", str(type_err))

        for weird_val in val_err_triggers:
            val_err = None
            try:
                _ = numenc.to_uint64(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "Illegal input: expected bytes of length 8, "
                "got {}.".format(len(weird_val)), str(val_err))


if __name__ == '__main__':
    unittest.main()
