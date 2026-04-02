// ==========================================
// vibration.c
// Python C Extension for vibration analysis
// ==========================================

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>

/*
==========================================
Helper: Validate input (list/tuple of floats)
------------------------------------------
- Ensures object is list or tuple
- Ensures all elements are float
- Converts to C double safely
==========================================
*/

static int validate_input(PyObject *data, Py_ssize_t *len) {
    if (!PyList_Check(data) && !PyTuple_Check(data)) {
        PyErr_SetString(PyExc_TypeError, "Input must be a list or tuple");
        return 0;
    }

    *len = PySequence_Size(data);

    if (*len == 0) {
        PyErr_SetString(PyExc_ValueError, "Input cannot be empty");
        return 0;
    }

    return 1;
}

/*
==========================================
peak_to_peak(data)
------------------------------------------
Mathematics:
max(data) - min(data)

Time Complexity: O(n)
==========================================
*/
static PyObject* peak_to_peak(PyObject *self, PyObject *args) {
    PyObject *data;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "O", &data))
        return NULL;

    if (!validate_input(data, &len))
        return NULL;

    double min, max, val;

    val = PyFloat_AsDouble(PySequence_GetItem(data, 0));
    min = max = val;

    for (Py_ssize_t i = 1; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        if (val < min) min = val;
        if (val > max) max = val;
    }

    return PyFloat_FromDouble(max - min);
}

/*
==========================================
rms(data)
------------------------------------------
Mathematics:
sqrt(sum(x^2) / n)

Time Complexity: O(n)
==========================================
*/
static PyObject* rms(PyObject *self, PyObject *args) {
    PyObject *data;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "O", &data))
        return NULL;

    if (!validate_input(data, &len))
        return NULL;

    double sum = 0.0, val;

    for (Py_ssize_t i = 0; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        sum += val * val;
    }

    return PyFloat_FromDouble(sqrt(sum / len));
}

/*
==========================================
std_dev(data)
------------------------------------------
Mathematics (Sample Std Dev):
sqrt( Σ(x - mean)^2 / (n - 1) )

Uses TWO-PASS method for stability:
1. Compute mean
2. Compute variance

Time Complexity: O(n)
==========================================
*/
static PyObject* std_dev(PyObject *self, PyObject *args) {
    PyObject *data;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "O", &data))
        return NULL;

    if (!validate_input(data, &len))
        return NULL;

    if (len < 2) {
        PyErr_SetString(PyExc_ValueError, "Need at least 2 values");
        return NULL;
    }

    double sum = 0.0, val;

    // First pass: mean
    for (Py_ssize_t i = 0; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        sum += val;
    }

    double mean = sum / len;

    // Second pass: variance
    double variance = 0.0;
    for (Py_ssize_t i = 0; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        double diff = val - mean;
        variance += diff * diff;
    }

    return PyFloat_FromDouble(sqrt(variance / (len - 1)));
}

/*
==========================================
above_threshold(data, threshold)
------------------------------------------
Counts values > threshold

Time Complexity: O(n)
==========================================
*/
static PyObject* above_threshold(PyObject *self, PyObject *args) {
    PyObject *data;
    double threshold;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "Od", &data, &threshold))
        return NULL;

    if (!validate_input(data, &len))
        return NULL;

    int count = 0;
    double val;

    for (Py_ssize_t i = 0; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        if (val > threshold) count++;
    }

    return PyLong_FromLong(count);
}

/*
==========================================
summary(data)
------------------------------------------
Returns:
{
  count, mean, min, max
}

Time Complexity: O(n)
==========================================
*/
static PyObject* summary(PyObject *self, PyObject *args) {
    PyObject *data;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "O", &data))
        return NULL;

    if (!validate_input(data, &len))
        return NULL;

    double sum = 0.0, val;
    double min, max;

    val = PyFloat_AsDouble(PySequence_GetItem(data, 0));
    min = max = val;
    sum = val;

    for (Py_ssize_t i = 1; i < len; i++) {
        val = PyFloat_AsDouble(PySequence_GetItem(data, i));
        sum += val;
        if (val < min) min = val;
        if (val > max) max = val;
    }

    double mean = sum / len;

    return Py_BuildValue(
        "{s:n,s:d,s:d,s:d}",
        "count", len,
        "mean", mean,
        "min", min,
        "max", max
    );
}

/*
==========================================
Method Table
==========================================
*/
static PyMethodDef VibrationMethods[] = {
    {"peak_to_peak", peak_to_peak, METH_VARARGS, "Peak-to-peak value"},
    {"rms", rms, METH_VARARGS, "Root mean square"},
    {"std_dev", std_dev, METH_VARARGS, "Sample standard deviation"},
    {"above_threshold", above_threshold, METH_VARARGS, "Count above threshold"},
    {"summary", summary, METH_VARARGS, "Basic statistics"},
    {NULL, NULL, 0, NULL}
};

/*
==========================================
Module Definition
==========================================
*/
static struct PyModuleDef vibrationmodule = {
    PyModuleDef_HEAD_INIT,
    "vibration",
    "Vibration analysis module",
    -1,
    VibrationMethods
};

/*
==========================================
Module Initialization
==========================================
*/
PyMODINIT_FUNC PyInit_vibration(void) {
    return PyModule_Create(&vibrationmodule);
}
