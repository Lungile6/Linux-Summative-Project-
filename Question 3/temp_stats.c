#include <Python.h>
#include <numpy/arrayobject.h>

// Documentation for min_temp:
// How it works: Iterates through the NumPy array of float temperatures and finds the minimum value.
// Memory usage considerations: It operates directly on the input NumPy array, avoiding extra memory allocation for data copying.
// Time complexity: O(n), where n is the number of temperature readings, as it iterates through the array once.
static PyObject *min_temp(PyObject *self, PyObject *args)
{
    PyArrayObject *in_array;

    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &in_array)) {
        return NULL;
    }

    float *data = (float *)PyArray_DATA(in_array);
    npy_intp num_readings = PyArray_SIZE(in_array);

    if (num_readings == 0) {
        PyErr_SetString(PyExc_ValueError, "Input array cannot be empty");
        return NULL;
    }

    float min_val = data[0];
    for (npy_intp i = 1; i < num_readings; i++) {
        if (data[i] < min_val) {
            min_val = data[i];
        }
    }

    return PyFloat_FromDouble(min_val);
}

// Documentation for max_temp:
// How it works: Iterates through the NumPy array of float temperatures and finds the maximum value.
// Memory usage considerations: It operates directly on the input NumPy array, avoiding extra memory allocation for data copying.
// Time complexity: O(n), where n is the number of temperature readings, as it iterates through the array once.
static PyObject *max_temp(PyObject *self, PyObject *args)
{
    PyArrayObject *in_array;

    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &in_array)) {
        return NULL;
    }

    float *data = (float *)PyArray_DATA(in_array);
    npy_intp num_readings = PyArray_SIZE(in_array);

    if (num_readings == 0) {
        PyErr_SetString(PyExc_ValueError, "Input array cannot be empty");
        return NULL;
    }

    float max_val = data[0];
    for (npy_intp i = 1; i < num_readings; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }

    return PyFloat_FromDouble(max_val);
}

// Documentation for avg_temp:
// How it works: Calculates the sum of all temperature readings and divides by the total number of readings.
// Memory usage considerations: It operates directly on the input NumPy array, avoiding extra memory allocation for data copying.
// Time complexity: O(n), where n is the number of temperature readings, as it iterates through the array once.
static PyObject *avg_temp(PyObject *self, PyObject *args)
{
    PyArrayObject *in_array;

    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &in_array)) {
        return NULL;
    }

    float *data = (float *)PyArray_DATA(in_array);
    npy_intp num_readings = PyArray_SIZE(in_array);

    if (num_readings == 0) {
        PyErr_SetString(PyExc_ValueError, "Input array cannot be empty");
        return NULL;
    }

    double sum = 0.0;
    for (npy_intp i = 0; i < num_readings; i++) {
        sum += data[i];
    }

    return PyFloat_FromDouble(sum / num_readings);
}

// Documentation for variance_temp:
// How it works: Calculates the sample variance using the formula: sum((x_i - mean)^2) / (n - 1).
// Memory usage considerations: It operates directly on the input NumPy array, avoiding extra memory allocation for data copying.
// Time complexity: O(n), where n is the number of temperature readings, as it iterates through the array twice (once for mean, once for sum of squares).
static PyObject *variance_temp(PyObject *self, PyObject *args)
{
    PyArrayObject *in_array;

    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &in_array)) {
        return NULL;
    }

    float *data = (float *)PyArray_DATA(in_array);
    npy_intp num_readings = PyArray_SIZE(in_array);

    if (num_readings < 2) {
        PyErr_SetString(PyExc_ValueError, "At least two readings are required to compute variance");
        return NULL;
    }

    // Calculate mean first
    double sum = 0.0;
    for (npy_intp i = 0; i < num_readings; i++) {
        sum += data[i];
    }
    double mean = sum / num_readings;

    // Calculate sum of squared differences for variance
    double sum_sq_diff = 0.0;
    for (npy_intp i = 0; i < num_readings; i++) {
        sum_sq_diff += (data[i] - mean) * (data[i] - mean);
    }

    return PyFloat_FromDouble(sum_sq_diff / (num_readings - 1));
}

// Documentation for count_readings:
// How it works: Returns the total number of elements in the input NumPy array.
// Memory usage considerations: It operates directly on the input NumPy array, avoiding extra memory allocation for data copying.
// Time complexity: O(1), as it directly accesses the size information from the NumPy array object.
static PyObject *count_readings(PyObject *self, PyObject *args)
{
    PyArrayObject *in_array;

    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &in_array)) {
        return NULL;
    }

    npy_intp num_readings = PyArray_SIZE(in_array);

    return PyLong_FromSsize_t(num_readings);
}

static PyMethodDef TempStatsMethods[] = {
    {"min_temp", min_temp, METH_VARARGS, "Returns the minimum temperature recorded."}, 
    {"max_temp", max_temp, METH_VARARGS, "Returns the maximum temperature recorded."}, 
    {"avg_temp", avg_temp, METH_VARARGS, "Returns the average (mean) temperature."}, 
    {"variance_temp", variance_temp, METH_VARARGS, "Returns the variance of the temperature readings (sample-based)."}, 
    {"count_readings", count_readings, METH_VARARGS, "Returns the total number of temperature readings."}, 
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef tempstatsmodule = {
    PyModuleDef_HEAD_INIT,
    "temp_stats",   /* name of module */
    "Module that provides temperature statistics functions.", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    TempStatsMethods
};

PyMODINIT_FUNC PyInit_temp_stats(void)
{
    import_array(); // Required for NumPy C-API
    return PyModule_Create(&tempstatsmodule);
}
