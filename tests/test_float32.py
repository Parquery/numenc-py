#!/usr/bin/env python3
# pylint: disable=missing-docstring
import unittest

import hypothesis
import hypothesis.strategies
import numenc

MIN_FLOAT32 = -3.402823466385288598117041834851e+38
MAX_FLOAT32 = 3.402823466385288598117041834851e+38


class TestEncodingFloat32(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.floats(
            min_value=MIN_FLOAT32, max_value=MAX_FLOAT32, width=32))
    def test_encode_decode_automatic(self, value: float):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_float32(numenc.from_float32(value)))

    def test_encode_order_corner_cases(self):
        # yapf: disable
        # pylint: disable=line-too-long
        table_smaller = [
            (-0.0, float('inf'), "-0 should be smaller than infinity "),
            (+0.0, float('inf'), "0 should be smaller than infinity "),
            (float('-inf'), -0.0, "negative infinity should be smaller than -0"),
            (float('-inf'), +0.0, "negative infinity should be smaller than +0"),
            (1.0, float('inf'), "1 should be smaller than infinity"),
            (float('-inf'), 1.0, "negative infinity should be smaller than 1"),
            (-1992.1210, -0.0, "-1992.1210 should be smaller than -0"),
            (-0.0, 1231111.123034, "-0 should be smaller than 1231111.123034"),
            (0.0, 1231111.123034, "0 should be smaller than 1231111.123034"),
        ]

        table_equals = [
            (float('inf'), float('inf'), "infinity should be equal to itself"),
            (float('-inf'), float('-inf'), "negative infinity should be equal to itself"),
            (+0.0, -0.0, "+0 should be equal to -0"),
            (0.0, +0.0, "0 should be equal to +0"),
            (0.0, -0.0, "0 should be equal to -0"),
        ]
        # yapf: enable
        errs = []
        for test_tuple in table_smaller:
            first_enc = numenc.from_float32(test_tuple[0])
            second_enc = numenc.from_float32(test_tuple[1])
            if not first_enc < second_enc:
                errs.append(test_tuple[2])

        for test_tuple in table_equals:
            first_enc = numenc.from_float32(test_tuple[0])
            second_enc = numenc.from_float32(test_tuple[1])
            if first_enc != second_enc:
                errs.append(test_tuple[2])

        self.assertEqual([], errs)

    @hypothesis.given(
        value1=hypothesis.strategies.floats(
            min_value=MIN_FLOAT32, max_value=MAX_FLOAT32, width=32),
        value2=hypothesis.strategies.floats(
            min_value=MIN_FLOAT32, max_value=MAX_FLOAT32, width=32))
    def test_encode_order(self, value1: float, value2: float):
        encoded1 = numenc.from_float32(value1)
        encoded2 = numenc.from_float32(value2)
        if value1 < value2:
            self.assertLess(encoded1, encoded2)
        elif value1 == value2:
            self.assertEqual(encoded1, encoded2)
        else:
            self.assertGreater(encoded1, encoded2)

    def test_encode_decode(self):
        # yapf: disable
        values = [
            float("-inf"),
            MIN_FLOAT32,
            -3223.330078125,
            -3,
            -0,
            0,
            3,
            4234523.5,
            MAX_FLOAT32,
            float("inf")
        ]

        expected = [
            b'\x00\x7f\xff\xff',
            b'\x00\x80\x00\x00',
            b'\x3a\xb6\x8a\xb7',
            b'\x3f\xbf\xff\xff',
            b'\x80\x00\x00\x00',
            b'\x80\x00\x00\x00',
            b'\xc0\x40\x00\x00',
            b'\xca\x81\x3a\x37',
            b'\xff\x7f\xff\xff',
            b'\xff\x80\x00\x00'
        ]
        # yapf: enable

        for i, value in enumerate(values):
            key = numenc.from_float32(value)
            self.assertEqual(expected[i], key)

            decoded = numenc.to_float32(key)
            self.assertEqual(value, decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLessEqual(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, b'\x01\x02']

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_float32(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, "
                "but got no error.".format(weird_val))

            self.assertEqual("Wrong input: expected 32-bit float.",
                             str(type_err))

    def test_decode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, 232, -1.23]
        val_err_triggers = [
            b'\x01', b'', b'\x01\x02\x02',
            b'\x01\x02\x01\x02\x01\x02\x01\x02\x02\x02'
        ]

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.to_float32(weird_val)
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
                _ = numenc.to_float32(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "Illegal input: expected bytes of length 4, got "
                "{}.".format(len(weird_val)), str(val_err))


if __name__ == '__main__':
    unittest.main()
