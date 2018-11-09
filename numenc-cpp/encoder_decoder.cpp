#include <Python.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

static const int8_t ONE_BYTE=1;
static const int8_t TWO_BYTES=2;
static const int8_t FOUR_BYTES=4;
static const int8_t EIGHT_BYTES=8;

static int8_t endianness_impl();

static const int8_t ENDIANNESS = endianness_impl();

union float_bytes {
    float val;
    unsigned char bytes[sizeof(float)];
};

union double_bytes {
    double val;
    unsigned char bytes[sizeof(double)];
};

// return 0 if the machine runs on big endian and
// 1 if machine runs on little endian.
static int8_t endianness_impl(void) {
    uint16_t number = 0x1;
    char* numPtr = (char* ) & number;
    return (int8_t)(numPtr[0] == 1);
}

static PyObject* from_int8(PyObject* self, PyObject* args) {
    int input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "i", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input type: expected integer.");
    }
    if (input < -128 || input > 127) {
        return PyErr_Format(PyExc_ValueError,
            "expected 8-bit signed integer (range [-128, 127]), "
            "got %d.", input);
    }

    char* buffer = (char* ) PyMem_RawMalloc(ONE_BYTE);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }
    buffer[0] = (char) input;
    // flip sign bit
    buffer[0] ^= 0x80;

    output = PyBytes_FromStringAndSize(buffer, ONE_BYTE);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_int8(PyObject* self, PyObject* args) {
    const char* input;
    int8_t decoded;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != ONE_BYTE) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            ONE_BYTE, count);
    }

    decoded = (int8_t)((unsigned char) input[0] ^ 0x80);
    output = Py_BuildValue("b", decoded);
    return output;
}

static PyObject* from_uint8(PyObject* self, PyObject* args) {
    int input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "i", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input type: expected integer.");
    }
    if (input < 0 || input > 255) {
        return PyErr_Format(PyExc_ValueError,
            "expected 8-bit unsigned integer (range [0, 255]), "
            "got %d.", input);
    }

    char* buffer = (char* ) PyMem_RawMalloc(ONE_BYTE);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }
    buffer[0] = (unsigned char) input;

    output = PyBytes_FromStringAndSize(buffer, ONE_BYTE);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_uint8(PyObject* self, PyObject* args) {
    const char* input;
    uint8_t decoded;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != ONE_BYTE) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            ONE_BYTE, count);
    }

    decoded = (uint8_t) input[0];
    output = Py_BuildValue("b", decoded);
    return output;
}

static PyObject* from_int16(PyObject* self, PyObject* args) {
    int16_t input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "h", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected signed 16-bit integer.");
    }

    char* buffer = (char* ) PyMem_RawMalloc(TWO_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < TWO_BYTES; i++) {
            buffer[i] = (input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < TWO_BYTES; i++) {
            buffer[TWO_BYTES - 1 - i] = (input >> (i * 8)) & 0xff;
        }
    }
    // flip sign bit
    buffer[0] ^= 0x80;

    output = PyBytes_FromStringAndSize(buffer, TWO_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_int16(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != TWO_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            TWO_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(TWO_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    buffer[0] = (unsigned char) input[0] ^ 0x80;
    for (int i = 1; i < TWO_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    int16_t decoded = 0;
    if (ENDIANNESS == 0) {
        decoded = (int16_t) buffer[0] | (int16_t) buffer[1] << 8;
    } else {
        decoded = (int16_t) buffer[1] | (int16_t) buffer[0] << 8;
    }

    output = Py_BuildValue("h", decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_uint16(PyObject* self, PyObject* args) {
    int input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "i", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input type: expected integer.");
    }
    if (input < 0 || input > 65535) {
        return PyErr_Format(PyExc_ValueError,
            "expected 16-bit unsigned integer (range [0, 65535]), "
            "got %d.", input);
    }

    uint16_t u_input = (uint16_t) input;

    char* buffer = (char* ) PyMem_RawMalloc(TWO_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < TWO_BYTES; i++) {
            buffer[i] = (u_input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < TWO_BYTES; i++) {
            buffer[TWO_BYTES - 1 - i] = (u_input >> (i * 8)) & 0xff;
        }
    }

    output = PyBytes_FromStringAndSize(buffer, TWO_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_uint16(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != TWO_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            TWO_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(TWO_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    for (int i = 0; i < TWO_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    uint16_t decoded = 0;
    if (ENDIANNESS == 0) {
        decoded = (uint16_t) buffer[0] | (uint16_t) buffer[1] << 8;
    } else {
        decoded = (uint16_t) buffer[1] | (uint16_t) buffer[0] << 8;
    }

    output = Py_BuildValue("H", decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_int32(PyObject* self, PyObject* args) {
    int32_t input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "i", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected signed 32-bit integer.");
    }

    char* buffer = (char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < FOUR_BYTES; i++) {
            buffer[i] = (input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < FOUR_BYTES; i++) {
            buffer[FOUR_BYTES - 1 - i] = (input >> (i * 8)) & 0xff;
        }
    }
    // flip sign bit
    buffer[0] ^= 0x80;

    output = PyBytes_FromStringAndSize(buffer, FOUR_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_int32(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != FOUR_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            FOUR_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    buffer[0] = (unsigned char) input[0] ^ 0x80;
    for (int i = 1; i < FOUR_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    int32_t decoded = 0;
    if (ENDIANNESS == 0) {
        for (int i = 0; i < FOUR_BYTES; i++) {
            decoded |= ((int32_t) buffer[i]) << (i * 8);
        }
    } else {
        for (int i = 0; i < FOUR_BYTES; i++) {
            decoded |= ((int32_t) buffer[FOUR_BYTES - 1 - i]) << (i * 8);
        }
    }

    output = Py_BuildValue("i", decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_uint32(PyObject* self, PyObject* args) {
    long long input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "L", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input type: expected integer.");
    }
    if (input < 0 || input > 4294967295) {
        return PyErr_Format(PyExc_ValueError,
            "expected 32-bit unsigned integer (range [0, 4294967295]),"
            " got %lld.", input);
    }

    uint32_t u_input = (uint32_t) input;

    char* buffer = (char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < FOUR_BYTES; i++) {
            buffer[i] = (u_input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < FOUR_BYTES; i++) {
            buffer[FOUR_BYTES - 1 - i] = (u_input >> (i * 8)) & 0xff;
        }
    }

    output = PyBytes_FromStringAndSize(buffer, FOUR_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_uint32(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != FOUR_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            FOUR_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    for (int i = 0; i < FOUR_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    uint32_t decoded = 0;
    if (ENDIANNESS == 0) {
        for (int i = 0; i < FOUR_BYTES; i++) {
            decoded |= ((uint32_t) buffer[i]) << (i * 8);
        }
    } else {
        for (int i = 0; i < FOUR_BYTES; i++) {
            decoded |= ((uint32_t) buffer[FOUR_BYTES - 1 - i]) << (i * 8);
        }
    }

    output = Py_BuildValue("I", decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_int64(PyObject* self, PyObject* args) {
    int64_t input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "L", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected signed 64-bit integer.");
    }

    char* buffer = (char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            buffer[i] = (input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            buffer[EIGHT_BYTES - 1 - i] = (input >> (i * 8)) & 0xff;
        }
    }
    // flip sign bit
    buffer[0] ^= 0x80;

    output = PyBytes_FromStringAndSize(buffer, EIGHT_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_int64(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != EIGHT_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            EIGHT_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    buffer[0] = (unsigned char) input[0] ^ 0x80;
    for (int i = 1; i < EIGHT_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    int64_t decoded = 0;
    if (ENDIANNESS == 0) {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            decoded |= ((int64_t) buffer[i]) << (i * 8);
        }
    } else {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            decoded |= ((int64_t) buffer[EIGHT_BYTES - 1 - i]) << (i * 8);
        }
    }
    output = PyLong_FromLongLong(decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_uint64(PyObject* self, PyObject* args) {
    PyObject* output;
    PyObject* input_obj;
    if (!PyArg_ParseTuple(args, "O", &input_obj)) {
        return PyErr_Format(PyExc_TypeError,
            "Error parsing the input as an object.");
    }

    unsigned long long input = PyLong_AsUnsignedLongLong(input_obj);
    if (input == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected unsigned 64-bit integer.");
    }

    char* buffer = (char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (ENDIANNESS == 0) {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            buffer[i] = (input >> (i * 8)) & 0xff;
        }
    } else {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            buffer[EIGHT_BYTES - 1 - i] = (input >> (i * 8)) & 0xff;
        }
    }

    output = PyBytes_FromStringAndSize(buffer, EIGHT_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_uint64(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != EIGHT_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            EIGHT_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    for (int i = 0; i < EIGHT_BYTES; i++) {
        buffer[i] = (unsigned char) input[i];
    }

    uint64_t decoded = 0;
    if (ENDIANNESS == 0) {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            decoded |= ((uint64_t) buffer[i]) << (i * 8);
        }
    } else {
        for (int i = 0; i < EIGHT_BYTES; i++) {
            decoded |= ((uint64_t) buffer[EIGHT_BYTES - 1 - i]) << (i * 8);
        }
    }
    output = PyLong_FromUnsignedLongLong(decoded);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_float32(PyObject* self, PyObject* args) {
    float input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "f", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected 32-bit float.");
    }

    union float_bytes f_bytes;
    f_bytes.val = input;

    char* buffer = (char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (input >= 0) {
        // positive number: set sign bit to 1
        if (ENDIANNESS == 0) {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[i] = f_bytes.bytes[i];
            }
        } else {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[FOUR_BYTES - 1 - i] = f_bytes.bytes[i];
            }
        }
        buffer[0] |= 0x80;
    } else {
        // negative number: flip all bits
        if (ENDIANNESS == 0) {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[i] = ~f_bytes.bytes[i];
            }
        } else {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[FOUR_BYTES - 1 - i] = ~f_bytes.bytes[i];
            }
        }
    }

    output = PyBytes_FromStringAndSize(buffer, FOUR_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_float32(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != FOUR_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            FOUR_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(FOUR_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (input[0] & 0x80) {
        // sign bit is 1: positive number or zero
        if (ENDIANNESS == 0) {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[i] = (unsigned char) input[i];
            }
            buffer[0] ^= 0x80;
        } else {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[FOUR_BYTES - 1 - i] = (unsigned char) input[i];
            }
            buffer[FOUR_BYTES - 1] ^= 0x80;
        }
    } else {
        // negative number
        if (ENDIANNESS == 0) {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[i] = ~((unsigned char) input[i]);
            }
        } else {
            for (int i = 0; i < FOUR_BYTES; i++) {
                buffer[FOUR_BYTES - 1 - i] = ~((unsigned char) input[i]);
            }
        }
    }

    union float_bytes f_bytes;

    for (int i = 0; i < FOUR_BYTES; i++) {
        f_bytes.bytes[i] = buffer[i];
    }

    output = Py_BuildValue("f", f_bytes.val);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* from_float64(PyObject* self, PyObject* args) {
    double input;
    PyObject* output;

    if (!PyArg_ParseTuple(args, "d", & input)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected 64-bit float.");
    }

    union double_bytes f_bytes;
    f_bytes.val = input;

    char* buffer = (char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (input >= 0) {
        // positive number: set sign bit to 1
        if (ENDIANNESS == 0) {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[i] = f_bytes.bytes[i];
            }
        } else {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[EIGHT_BYTES - 1 - i] = f_bytes.bytes[i];
            }
        }
        buffer[0] |= 0x80;
    } else {
        // negative number: flip all bits
        if (ENDIANNESS == 0) {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[i] = ~f_bytes.bytes[i];
            }
        } else {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[EIGHT_BYTES - 1 - i] = ~f_bytes.bytes[i];
            }
        }
    }

    output = PyBytes_FromStringAndSize(buffer, EIGHT_BYTES);
    PyMem_RawFree(buffer);
    return output;
}

static PyObject* to_float64(PyObject* self, PyObject* args) {
    const char* input;
    PyObject* output;
    int count;

    if (!PyArg_ParseTuple(args, "y#", & input, & count)) {
        return PyErr_Format(PyExc_TypeError,
            "Wrong input: expected bytes.");
    }

    if (count != EIGHT_BYTES) {
        return PyErr_Format(PyExc_ValueError,
            "Illegal input: expected bytes of length %d, got %d.",
            EIGHT_BYTES, count);
    }

    unsigned char* buffer = (unsigned char* ) PyMem_RawMalloc(EIGHT_BYTES);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    if (input[0] & 0x80) {
        // sign bit is 1: positive number or zero
        if (ENDIANNESS == 0) {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[i] = (unsigned char) input[i];
            }
            buffer[0] ^= 0x80;
        } else {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[EIGHT_BYTES - 1 - i] = (unsigned char) input[i];
            }
            buffer[EIGHT_BYTES - 1] ^= 0x80;
        }
    } else {
        // negative number
        if (ENDIANNESS == 0) {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[i] = ~((unsigned char) input[i]);
            }
        } else {
            for (int i = 0; i < EIGHT_BYTES; i++) {
                buffer[EIGHT_BYTES - 1 - i] = ~((unsigned char) input[i]);
            }
        }
    }

    union double_bytes d_bytes;

    for (int i = 0; i < EIGHT_BYTES; i++) {
        d_bytes.bytes[i] = buffer[i];
    }

    output = Py_BuildValue("d", d_bytes.val);
    PyMem_RawFree(buffer);
    return output;
}

static PyMethodDef EncdecMethods[] = {
    {
        "from_int8",
        from_int8,
        METH_VARARGS,
        "Convert an 8-bit signed integer to sortable bytes"
    },
    {
        "to_int8",
        to_int8,
        METH_VARARGS,
        "Convert bytes back to a signed 8-bit integer"
    },
    {
        "from_uint8",
        from_uint8,
        METH_VARARGS,
        "Convert an 8-bit unsigned integer to sortable bytes"
    },
    {
        "to_uint8",
        to_uint8,
        METH_VARARGS,
        "Convert bytes back to an unsigned 8-bit integer"
    },

    {
        "from_int16",
        from_int16,
        METH_VARARGS,
        "Convert a 16-bit signed integer to sortable bytes"
    },
    {
        "to_int16",
        to_int16,
        METH_VARARGS,
        "Convert bytes back to a signed 16-bit integer"
    },
    {
        "from_uint16",
        from_uint16,
        METH_VARARGS,
        "Convert a 16-bit unsigned integer to sortable bytes"
    },
    {
        "to_uint16",
        to_uint16,
        METH_VARARGS,
        "Convert bytes back to an unsigned 16-bit integer"
    },

    {
        "from_int32",
        from_int32,
        METH_VARARGS,
        "Convert a 32-bit signed integer to sortable bytes"
    },
    {
        "to_int32",
        to_int32,
        METH_VARARGS,
        "Convert bytes back to a signed 32-bit integer"
    },
    {
        "from_uint32",
        from_uint32,
        METH_VARARGS,
        "Convert a 32-bit unsigned integer to sortable bytes"
    },
    {
        "to_uint32",
        to_uint32,
        METH_VARARGS,
        "Convert bytes back to an unsigned 32-bit integer"
    },

    {
        "from_int64",
        from_int64,
        METH_VARARGS,
        "Convert a signed 64-bit integer to sortable bytes"
    },
    {
        "to_int64",
        to_int64,
        METH_VARARGS,
        "Convert bytes back to a signed 64-bit integer"
    },
    {
        "from_uint64",
        from_uint64,
        METH_VARARGS,
        "Convert an unsigned 64-bit integer to sortable bytes"
    },
    {
        "to_uint64",
        to_uint64,
        METH_VARARGS,
        "Convert bytes back to an unsigned 64-bit integer"
    },

    {
        "from_float32",
        from_float32,
        METH_VARARGS,
        "Convert a 32-bit float to sortable bytes"
    },
    {
        "to_float32",
        to_float32,
        METH_VARARGS,
        "Convert bytes back to a 32-bit float"
    },

    {
        "from_float64",
        from_float64,
        METH_VARARGS,
        "Convert a 64-bit float to sortable bytes"
    },
    {
        "to_float64",
        to_float64,
        METH_VARARGS,
        "Convert bytes back to a 64-bit float"
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

static struct PyModuleDef cModPyDem = {
    PyModuleDef_HEAD_INIT,
    "numenc",
    "Encode and decode numbers to sortable bytes",
    -1,
    EncdecMethods
};

PyMODINIT_FUNC
PyInit_numenc(void) {
    return PyModule_Create( & cModPyDem);
}
