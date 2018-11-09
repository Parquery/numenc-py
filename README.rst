Pynumenc
========

Pynumenc is a Python3 library to translate numbers to and from sortable bytes.

We frequently had to encode numbers (integers as well as floating points) and
store them in key/value stores in a sortable order. The current Python
encoding of numbers into bytes lacked:

* order. Python uses Two's Complement to represent integers and IEEE-754 to
  represent floats, respectively. These representations do not maintain order
  of bytes since the representation of negative numbers comes after the
  representation of the positive numbers (see below for examples).

* speed. Python does not currently allow for manipulating individual bits in
  bytes. For example, to invert a part of bytes, you need to slice it,
  convert it to int, apply bit transformations on an int, convert it back to
  bytes and join the resulting slice with the prefix and suffix. This worked
  too slow for our applications (about 100x slower) so we implemented the
  encoding in C++.

The library contains conversion functions from/to the following types:

=========   ========  ====  =====  ==================================  =====================================
Specifier   Signing   Bits  Bytes  Minimum Value                       Maximum Value
=========   ========  ====  =====  ==================================  =====================================
int8        Signed    8     1      -2^7 (-128)                         2^7 - 1  (127)
uint8       Unsigned  8     1      0                                   2^8 - 1  (255)
int16       Signed    16    2      -2^15 (-32,768)                      2^15 - 1 (32,767)
uint16      Unsigned  16    2      0                                   2^16 - 1 (65,535)
int32       Signed    32    4      -2^31 (-2,147,483,648)              2^31 - 1 (2,147,483,647)
uint32      Unsigned  32    4      0                                   2^32 - 1 (4,294,967,295)
int64       Signed    64    8      -2^63 (-9,223,372,036,854,775,808)  2^63 - 1 (9,223,372,036,854,775,807)
uint64      Unsigned  64    8      0                                   2^64 - 1 (18,446,744,073,709,551,615)
float32     Signed    32    8      -3.402823466385288598117041834e+38  3.4028234663852885981170418348451e+38
float64     Signed    64    8      -1.79769313486231570814527423e+308  1.797693134862315708145274237317e+308
=========   ========  ====  =====  ==================================  =====================================

Unlike the default bit representation of integers and floats, the above
functions use bit-manipulation techniques to guarantee that not only mappings
are injective, but that the resulting bytes preserve the order of the numbers,
that is:

* encoding(a) < encoding(b) ⇔ a < b
* encoding(a) = encoding(b) ⇔ a = b

The rules above imply that we do not follow IEEE-754's standard of treating
negative zero as smaller than positive zero: we treat them as the same number
(more on this below).

How it works
============

The "traditional" encoding and decoding of numbers works as follows:

* integers are stored in 1 or more bytes, where the first bit of the first byte
  determines the sign, and the rest of the bits determine the number. See
  `two's complement <https://en.wikipedia.org/wiki/Two%27s_complement>`_.
* floating-point numbers are encoded following
  `the IEEE-754 standard <https://en.wikipedia.org/wiki/IEEE_754>`_,
  according to which finite numbers are specified as a sign bit, a coefficient
  and an exponent. Special cases of the IEEE 754 format include two signed
  zeroes (+0, -0), two infinities (+∞ and −∞) and two NaN values.

Neither of these encodings preserve the order of the numbers in the
corresponding bytestring: integers negative numbers appear lexicographically
after positive ones (the first bit being responsible for the sign and equals
to one if the number is negative), and floating-point encoded values are
sorted decreasingly for negative input.

Upon storage, the order of the bytes array for both floating-point and integer
numbers depends on the machine's
`endianness <https://en.wikipedia.org/wiki/Endianness>`_:

* on Little Endian machines, the Most Significant Byte (MSB) is the last of the
  array;
* on Big Endian machines, the MSB is the first.

Our encoding scheme works as follows:

* integers are first encoded in a byte array following the machine's endianness
  and if the machine is Little Endian, the array is mirrored; then, the
  first byte (which will now be the MSB irregardless of the machine's
  endianness) is XOR'd with 10000000, hence the sign bit is flipped.
  For example, for 8-bit integers, the result of two's complement is

  [10000000 (-128), 10000001 (-127), ..., 11111111 (-1), 00000000 (0), 00000001 (1), ..., 01111111 (127)].

  Applying our encoding scheme will result in the ordering

  [00000000 (-128), 00000001 (-127), ..., 01111111 (-1), 10000000 (0), 10000001 (1), ..., 11111111 (127)].

* floating-point numbers are encoded in a byte array following the machine's
  endianness and then, if the machine is Little Endian, the array is
  mirrored; then, if the number is non-negative, the first byte (which will now
  be the MSB irregardless of the machine's endianness) is OR'd with 0x80
  (binary 10000000), hence the sign bit is set to 1 (treating the two signed
  zeroes as one); otherwise, the whole byte array is negated, which results
  in flipping all bits and thus reverse-ordering negative numbers. Plus and
  minus infinity can be encoded in this scheme and will come after and before
  all other byte arrays, respectively.

Usage
=====

As a Python library
-------------------

To use the encoding and decoding functions, you need to invoke one of the
``numenc.from_TYPE()`` and ``numenc.to_TYPE()`` functions, replacing TYPE
with any one of the types listed above. For example:

.. code-block:: python

    >>> import numenc

    >>> numenc.from_int32(1200)
    b'\x80\x00\x04\xb0'
    >>> numenc.to_int32(b'\x80\x00\x04\xb0')
    1200
    >>> numenc.from_int32("a string")
    Traceback (most recent call last):
     ...
    TypeError: Wrong input: expected signed 32-bit integer.

    # negative input throws ValueError for unsigned types
    >>> numenc.from_uint8(-1)
    Traceback (most recent call last):
     ...
    ValueError: expected 8-bit unsigned integer (range [0, 255]), got -1.

    >>> numenc.from_int64(-999999)
    b'\x7f\xff\xff\xff\xff\xf0\xbd\xc1'
    >>> numenc.to_int64(b'\x7f\xff\xff\xff\xff\xf0\xbd\xc1')
    -999999

    # small rounding differences are possible, as Python (normally)
    # uses float64 internally
    >>> numenc.from_float32(-23.44443)
    b'>Dq\xce'
    >>> numenc.to_float32(b'>Dq\xce')
    -23.444429397583008

    >>> numenc.from_float64(-23.44443)
    b'?\xc8\x8e9\xd5\xe4\xa3\x82'
    >>> numenc.to_float64(b'?\xc8\x8e9\xd5\xe4\xa3\x82')
    -23.44443

    >>> numenc.from_float32(float("-inf"))
    b'\x00\x7f\xff\xff'
    >>> numenc.from_float32(float("inf"))
    b'\xff\x80\x00\x00'


As a command line tool
----------------------
You can experiment with numenc on the command line by running the executable
pynumenc, which wraps the numenc library. For convenience, input and output
of type bytes is treated as hexadecimal strings (e.g., `b'\xde\xad\xbe\xef'`
is "deadbeef"). To run the executable, specify a conversion function
(``to_(TYPE)`` or ``from_(TYPE)``) and a value as positional arguments. For
example:

.. code-block:: bash

    pynumenc to_float32 80000000
    result: 0.0

    pynumenc from_uint8 45
    result: 2d


Installation
============

* Create a virtual environment:

.. code-block:: bash

    python3 -m venv venv3

* Activate it:

.. code-block:: bash

    source venv3/bin/activate

* Install pynumenc with pip:

.. code-block:: bash

    pip3 install pynumenc

Development
===========

* Check out the repository.

* In the repository root, create the virtual environment:

.. code-block:: bash

    python3 -m venv venv3

* Activate the virtual environment:

.. code-block:: bash

    source venv3/bin/activate

* Install the development dependencies:

.. code-block:: bash

    pip3 install -e .[dev]

* Run `precommit.py` to execute pre-commit checks locally.

Versioning
==========
We follow `Semantic Versioning <http://semver.org/spec/v1.0.0.html>`_.
The version X.Y.Z indicates:

* X is the major version (backward-incompatible),
* Y is the minor version (backward-compatible), and
* Z is the patch version (backward-compatible bug fix).