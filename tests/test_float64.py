#!/usr/bin/env python3
# pylint: disable=missing-docstring
import unittest

import hypothesis
import hypothesis.strategies
import numenc

MIN_FLOAT64 = -1.797693134862315708145274237317e+308
MAX_FLOAT64 = 1.797693134862315708145274237317e+308


class TestEncodingFloat64(unittest.TestCase):
    @hypothesis.given(
        hypothesis.strategies.floats(
            min_value=MIN_FLOAT64, max_value=MAX_FLOAT64))
    def test_encode_decode_automatic(self, value: float):
        hypothesis.assume(value)
        self.assertEqual(value, numenc.to_float64(numenc.from_float64(value)))

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
            first_enc = numenc.from_float64(test_tuple[0])
            second_enc = numenc.from_float64(test_tuple[1])
            if not first_enc < second_enc:
                errs.append(test_tuple[2])

        for test_tuple in table_equals:
            first_enc = numenc.from_float64(test_tuple[0])
            second_enc = numenc.from_float64(test_tuple[1])
            if first_enc != second_enc:
                errs.append(test_tuple[2])

        self.assertEqual([], errs)

    @hypothesis.given(
        value1=hypothesis.strategies.floats(
            min_value=MIN_FLOAT64, max_value=MAX_FLOAT64),
        value2=hypothesis.strategies.floats(
            min_value=MIN_FLOAT64, max_value=MAX_FLOAT64))
    def test_encode_order(self, value1: float, value2: float):
        encoded1 = numenc.from_float64(value1)
        encoded2 = numenc.from_float64(value2)
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
            MIN_FLOAT64,
            -3425243.1221,
            -3,
            0,
            -0,
            3,
            4234523.5,
            MAX_FLOAT64,
            float("inf")
        ]

        expected = [
            b"\x00\x0f\xff\xff\xff\xff\xff\xff",
            b"\x00\x10\x00\x00\x00\x00\x00\x00",
            b"\x3e\xb5\xde\x12\x70\x5f\x06\xf6",
            b"\x3f\xf7\xff\xff\xff\xff\xff\xff",
            b"\x80\x00\x00\x00\x00\x00\x00\x00",
            b"\x80\x00\x00\x00\x00\x00\x00\x00",
            b"\xc0\x08\x00\x00\x00\x00\x00\x00",
            b"\xc1\x50\x27\x46\xe0\x00\x00\x00",
            b"\xff\xef\xff\xff\xff\xff\xff\xff",
            b"\xff\xf0\x00\x00\x00\x00\x00\x00"
        ]
        # yapf: enable

        for i, value in enumerate(values):
            key = numenc.from_float64(value)
            self.assertEqual(expected[i], key)

            decoded = numenc.to_float64(key)
            self.assertEqual(value, decoded)

        for i, _ in enumerate(expected):
            if i > 0:
                self.assertLessEqual(expected[i - 1], expected[i])

    def test_encode_exceptions(self):
        type_err_triggers = ["some string", ('1', '2'), [], {}, b'\x01\x02']

        for weird_val in type_err_triggers:
            type_err = None
            try:
                _ = numenc.from_float64(weird_val)
            except TypeError as tperr:
                type_err = tperr

            self.assertIsNotNone(
                type_err,
                msg="excepted error thrown for input {!r}, "
                "but got no error.".format(weird_val))

            self.assertEqual("Wrong input: expected 64-bit float.",
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
                _ = numenc.to_float64(weird_val)
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
                _ = numenc.to_float64(weird_val)
            except ValueError as verr:
                val_err = verr

            self.assertIsNotNone(
                val_err,
                msg="excepted error thrown for input {!r}, but got no error.".
                format(weird_val))

            self.assertEqual(
                "Illegal input: expected bytes of length 8, got "
                "{}.".format(len(weird_val)), str(val_err))


if __name__ == '__main__':
    unittest.main()
