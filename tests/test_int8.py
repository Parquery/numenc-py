#!/usr/bin/env python3
# pylint: disable=missing-docstring
import unittest

import hypothesis
import hypothesis.strategies
import numenc

MIN_SIGNED_INT8 = -128
MAX_SIGNED_INT8 = 127
MAX_UNSIGNED_INT8 = 255


class TestEncodingInt8(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT8, max_value=MAX_SIGNED_INT8))
    def test_encode_decode_automatic(self, value: int):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_int8(numenc.from_int8(value)))

    @hypothesis.given(
        value1=hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT8, max_value=MAX_SIGNED_INT8),
        value2=hypothesis.strategies.integers(
            min_value=MIN_SIGNED_INT8, max_value=MAX_SIGNED_INT8))
    def test_encode_order(self, value1: int, value2: int):
        encoded1 = numenc.from_int8(value1)
        encoded2 = numenc.from_int8(value2)
        if value1 < value2:
            self.assertLess(encoded1, encoded2)
        elif value1 == value2:
            self.assertEqual(encoded1, encoded2)
        else:
            self.assertGreater(encoded1, encoded2)

    def test_encode_decode(self):
        # yapf: disable
        nums = [
            MIN_SIGNED_INT8,
            -32,
            0,
            89,
            MAX_SIGNED_INT8
        ]

        expected = [
            b'\x00',
            b'\x60',
            b'\x80',
            b'\xd9',
            b'\xff'
        ]
        # yapf: enable

        for i, num in enumerate(nums):
            key = numenc.from_int8(num)
            self.assertEqual(expected[i], key)

            num_decoded = numenc.to_int8(key)
            self.assertEqual(num, num_decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLess(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, b'\x01\x02']
        val_err_triggers = [-2222222, -129, 128]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_int8(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, "
                "but got no error.".format(weird_val))

            self.assertEqual("Wrong input type: expected integer.",
                             str(type_err))

        for weird_val in val_err_triggers:
            val_err = None
            try:
                _ = numenc.from_int8(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "expected 8-bit signed integer (range "
                "[-128, 127]), got {}.".format(weird_val), str(val_err))

    def test_decode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, 232, -1.23]
        val_err_triggers = [b'\x01\x02', b'', b'\x01\x02\x02\x02']

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.to_int8(weird_val)
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
                _ = numenc.to_int8(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "Illegal input: expected bytes of length 1, got "
                "{}.".format(len(weird_val)), str(val_err))


class TestEncodingUInt8(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT8))
    def test_encode_decode_automatic(self, value: int):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_uint8(numenc.from_uint8(value)))

    @hypothesis.given(
        value1=hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT8),
        value2=hypothesis.strategies.integers(
            min_value=0, max_value=MAX_UNSIGNED_INT8))
    def test_encode_order(self, value1: int, value2: int):
        encoded1 = numenc.from_uint8(value1)
        encoded2 = numenc.from_uint8(value2)
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
            96,
            128,
            217,
            MAX_UNSIGNED_INT8
        ]

        expected = [
            b'\x00',
            b'\x60',
            b'\x80',
            b'\xd9',
            b'\xff'
        ]
        # yapf: enable

        for i, num in enumerate(nums):
            key = numenc.from_uint8(num)
            self.assertEqual(expected[i], key)

            num_decoded = numenc.to_uint8(key)
            self.assertEqual(num, num_decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLess(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = [
            "some string", 2.344, ('1', '2'), [], {}, b'\x01\x02'
        ]
        val_err_triggers = [-2222222, -1, 256]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_uint8(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual("Wrong input type: expected integer.",
                             str(type_err))

        for weird_val in val_err_triggers:
            val_err = None
            try:
                _ = numenc.from_uint8(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "expected 8-bit unsigned integer (range [0, 255]),"
                " got {}.".format(weird_val), str(val_err))

    def test_decode_exceptions(self):
        type_err_triggers = [
            "some string", 2.344, ('1', '2'), [], {}, 232, -1.23
        ]
        val_err_triggers = [b'\x01\x01', b'', b'\x01\x02\x02\x02']

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.to_uint8(weird_val)
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
                _ = numenc.to_uint8(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "Illegal input: expected bytes of length 1, "
                "got {}.".format(len(weird_val)), str(val_err))


if __name__ == '__main__':
    unittest.main()
