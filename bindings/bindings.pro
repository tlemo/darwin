
TEMPLATE = subdirs

PYTHON_INCLUDE = $$(PYTHON_INCLUDE)
isEmpty(PYTHON_INCLUDE) {
    message('PYTHON_INCLUDE not defined, Python bindings will not be built')
} else {
    message('Including Python bindings (PYTHON_INCLUDE="$$PYTHON_INCLUDE")')
    SUBDIRS += python
}

