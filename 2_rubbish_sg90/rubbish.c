#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rubbish_Init(void)
{
    Py_Initialize();
    
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));
}

void rubbish_Finalize(void)
{
    Py_Finalize();
}

char *alicloud_rubbish_category(char *category)
{
    PyObject *pModule = PyImport_ImportModule("rubbish");
    if (!pModule)
    {
        PyErr_Print();
        printf("Error: failed to load rubbish.py\n");
        goto FAILED_MODULE;
    }
    
    PyObject *pFunc = PyObject_GetAttrString(pModule, "alicloud_classify_Rubbish");
    if (!pFunc)
    {
        PyErr_Print();
        printf("Error: failed to load classify_Rubbish\n");
        goto FAILED_FUNC;
    }
		
    PyObject *pValue = PyObject_CallObject(pFunc, NULL);
	if (!pValue)
    {
        PyErr_Print();
        printf("Error: function call failed\n");
        goto FAILED_VALUE;
    }
		
	char *result = NULL;
	if(!PyArg_Parse(pValue, "s", &result))
    {
        PyErr_Print();
        printf("Error: parse failed\n");
        goto FAILED_RESULT;
    }
    
    category = (char *)malloc(sizeof(char) * (strlen(result) + 1));
    memset(category, '\0', (strlen(result) + 1));
    strncpy(category, result, (strlen(result) + 1));
    
FAILED_RESULT:
    Py_DECREF(pValue);
FAILED_VALUE:
    Py_DECREF(pFunc);
FAILED_FUNC:
    Py_DECREF(pModule);
FAILED_MODULE:
    return category;
}

#if 0

void rubbish_Init(void)
{
    Py_Initialize();
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));
}
void rubbish_Finalize(void)
{
    Py_Finalize();
}
char *alicloud_rubbish_category(char *category)
{
    PyObject *pModule = PyImport_ImportModule("rubbish");
    if (!pModule)
    {
        PyErr_Print();
        printf("Error: failed to load rubbish.py\n");
        goto FAILED_MODULE;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "alibabacloud_garbage");
    if (!pFunc)
    {
        PyErr_Print();
        printf("Error: failed to load alibabacloud_garbage\n");
        goto FAILED_FUNC;
    }
    PyObject *pValue = PyObject_CallObject(pFunc, NULL);
    if (!pValue)
    {
        PyErr_Print();
        printf("Error: function call failed\n");
        goto FAILED_VALUE;
    }
    char *result = NULL;
    if (!PyArg_Parse(pValue, "s", &result))
    {
        PyErr_Print();
        printf("Error: parse failed");
        goto FAILED_RESULT;
    }
    category = (char *)malloc(sizeof(char) * (strlen(result) + 1));
    memset(category, 0, (strlen(result) + 1));
    strncpy(category, result, (strlen(result) + 1));
FAILED_RESULT:
    Py_DECREF(pValue);
FAILED_VALUE:
    Py_DECREF(pFunc);
FAILED_FUNC:
    Py_DECREF(pModule);
FAILED_MODULE:
    return category;
}

#endif