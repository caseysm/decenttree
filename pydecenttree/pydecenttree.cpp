//
//On Windows it might just be <Python.h>.  But on OS it's 
//a framework. The usual way around this is to #include <Python/Python.h>
//on Mac OS.  But I've hacked the -I parameter in CMakeLists.txt instead.
//-James B. 10-Jul-2022.
//
#define PY3K
#include <Python.h>              //yes, even on Mac OS.
#undef   USE_VECTORCLASS_LIBRARY
#define  USE_VECTORCLASS_LIBRARY 1 //Woo!

#define PY_ARRAY_UNIQUE_SYMBOL MY_PyArray_API 
#include <numpy/arrayobject.h>   //for numpy helper functions

#include <starttree.h>

class ScopedPyObjectPtr {
public:
    PyObject *ptr;
    ScopedPyObjectPtr(): ptr(nullptr) {}
    ScopedPyObjectPtr(PyObject* rhs) : ptr(rhs) {}
    ~ScopedPyObjectPtr() {
        if (ptr!=nullptr) {
            Py_DECREF(ptr);
        }
    }
    bool operator==(const ScopedPyObjectPtr& rhs) { return ptr == rhs.ptr; }
    bool operator==(const PyObject* rhs) { return ptr == rhs; }
    operator  PyObject*() const { return ptr; }
    PyObject* getPtr()    const { return ptr; }
    bool      isNull()    const { return ptr==nullptr; }
};

bool appendStrVector(PyObject* append_me, StrVector& to_me) {
    #if (PY_MAJOR_VERSION >= 3)
        ScopedPyObjectPtr str ( PyObject_Str(append_me) );
    #else
        ScopedPyObjectPtr str ( PyObject_Unicode(append_me) );
    #endif

    if (str.isNull()) {
        return false;
    }
    Py_ssize_t  len  = 0;
    char const* utf8 = PyUnicode_AsUTF8AndSize(str, &len);
    if (utf8!=nullptr) {
        to_me.emplace_back(utf8, len);
    }
    return (utf8!=nullptr);
}

bool isVectorOfString(const char* vector_name, PyObject*          sequence_arg,
                      StrVector&  sequences,   std::stringstream& complaint) {
    if (sequence_arg==nullptr) {
        complaint << vector_name << " was not supplied.";
        return false;
    }
    ScopedPyObjectPtr seq = PySequence_List(sequence_arg);
    if (seq==nullptr) {
        complaint << vector_name << " is not a sequence.";
        return false;
    }
    int number_of_sequences = PySequence_Fast_GET_SIZE(seq.getPtr());
    for (int i=0; i<number_of_sequences; ++i) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq.getPtr(), i);
        if (item==nullptr) {
            complaint << vector_name << " could not access item " << i << ".";
            return false;
        }
        if (!appendStrVector(item, sequences)) {
            complaint << vector_name << " could not convert item " << i << " to string.";
            return false;
        }
    }
    return true;
}

bool appendDoubleVector(PyObject* append_me, DoubleVector& to_me) {
    double float_val = PyFloat_AsDouble(append_me);
    if (float_val==-1.0 && PyErr_Occurred()) {
        return false;
    }
    to_me.emplace_back(float_val);
    return true;
}

bool appendDoublesToVector(const std::string& row_vector_name, PyObject* seq_for_row, 
                           DoubleVector& doubles, size_t& row_width_here, 
                           std::stringstream& complaint ) {
    //Assumptions: 1. seq_for_row is a sequence (it always is, when called
    //                from isVectorOfDouble)
    //             2. Caller owns req_for_row and controls its reference count.
    //             3. row_width_here has already been initialized (and will
    //                be incremented, for each item read from seq_for_row)
    //
    int number_of_items = PySequence_Fast_GET_SIZE(seq_for_row);
    for (int i=0; i<number_of_items; ++i) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq_for_row, i);
        if (item==nullptr) {
            complaint << row_vector_name << " could not access item " << i << ".";
            return false;
        }
        if (PySequence_Check(item)==1) {
            complaint << " item " << i << " of " << row_vector_name 
                      << " was a sequence (not allowed).";
        }
        else if (!appendDoubleVector(item, doubles)) {
            complaint << row_vector_name << ": could not convert item " 
                      << i << " to double.";
            return false;
        }
        ++row_width_here;
    }
    return true;
}

bool isVectorOfDouble(const char*   vector_name,   PyObject* vector_arg,
                      DoubleVector& doubles,       const double*& element_data,
                      size_t&       element_count, std::stringstream& complaint) {
    size_t row_count = 0; //Number of row sequences read
    size_t row_width = 0; //Width of all row sequences seen so far
    element_data  = nullptr;
    element_count = 0;
    if (vector_arg==nullptr) {
        complaint << vector_name << " was not supplied.";
        return false;
    }
    ScopedPyObjectPtr seq = PySequence_List(vector_arg);
    if (seq==nullptr) {
        complaint << vector_name << " is not a sequence.";
        return false;
    }
    int number_of_items = PySequence_Fast_GET_SIZE(seq.getPtr());
    for (int i=0; i<number_of_items; ++i) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq.getPtr(), i);
        if (item==nullptr) {
            complaint << vector_name << " could not access item " << i << ".";
            return false;
        }
        if (PySequence_Check(item)==1) {
            ScopedPyObjectPtr seq_for_row = PySequence_List(item);
            //Nested sequence
            std::stringstream row_seq_name;
            size_t            row_width_here=0;
            row_seq_name << "row [" << i << "] of " << vector_name;
            std::string row_vector_name = row_seq_name.str();
            if (!appendDoublesToVector(row_vector_name, seq_for_row, 
                                       doubles, row_width_here, complaint )) {
                return false;
            }
            if (row_count==0) {
                if (row_width_here<doubles.size()) {
                    complaint << "can't mix scalars and row vectors"
                              << " in " << vector_name << ".";
                    return false;
                }
                row_width = row_width_here;
            }
            ++row_count;
            if (row_width!=row_width_here) {
                complaint << row_vector_name << " has rank "
                    << row_width_here << " that differs with"
                    << " the rank (" << row_width << ") of previous rows"
                    << "\n";
                return false;
            }
        }
        else if (row_count!=0) {
            complaint << "can't mix row vectors and scalars"
                        << " in " << vector_name << ".";
            return false;
        }
        else if (!appendDoubleVector(item, doubles)) {
            complaint << vector_name << " could not convert item " 
                      << i << " to double.";
            return false;
        } 
    }

    element_data  = doubles.data();
    element_count = doubles.size();
    return true;
}

bool isMatrix(PyObject* arg) {
    return PyArray_API!=nullptr && PyArray_Check(arg)!=0;
}

bool isMatrixOfDouble(const char*        matrix_name,  PyObject* possible_matrix, 
                      const double*&     element_data, size_t&   element_count, 
                      std::stringstream& complaint) {            
    element_count = 0;
    if (!isMatrix(possible_matrix)) {
        complaint << matrix_name << " matrix"
                  << " is not a matrix of type Float";
        return false;
    }

    PyArrayObject* matrix = reinterpret_cast<PyArrayObject*>(possible_matrix);
    if (matrix->descr->type_num != NPY_DOUBLE) {
        complaint << matrix_name << " matrix"
                  << " is not a matrix of type Float";
        return false;
    }

    auto dimensions = matrix->nd;
    if (dimensions < 1 || 2<dimensions) {
        complaint << matrix_name << "matrix"
                  << " has " << dimensions << " dimensions"
                  << " (only 1 and 2 dimensional matrices are allowed).";
        return false;                  
    }
    element_count = 1;
    for (int d = 0; d < dimensions; ++d) {
        element_count *= matrix->dimensions[d];
    }
    element_data = reinterpret_cast<double*> PyArray_DATA(matrix);
    return true;
}

bool obeyThreadCount(int number_of_threads, std::stringstream& complaint) {
    #ifdef _OPENMP
        std::cout <<"OpenMP is defined\n";
        if (0<number_of_threads) {
            int maxThreadCount = omp_get_max_threads();
            if (maxThreadCount < number_of_threads ) {
                //For now, don't care.  Don't even warn about it.
                //Some day, maybe write to complaint and return false
            } else {
                omp_set_num_threads(number_of_threads);
            }
        }
        //If number of threads < 1 use the OMP default.
    #else
        //Ignore it.  Some day, maybe complain.
    #endif
    return true;
}

PyObject* StringToPythonString(const std::string& convert_me) {
    #if PY_MAJOR_VERSION >= 3
        return PyUnicode_FromString(convert_me.c_str());
    #else
        return PyString_FromString(convert_me.c_str());
    #endif
}

static PyObject* pydecenttree_constructTree(PyObject* self, PyObject* args, 
                                            PyObject* keywords)
{
    const char* argument_names[] = {
        "algorithm", "sequences", "distances", 
        "number_of_threads", "precision", 
        "verbosity", nullptr
    };
    char*          algorithm_name    = nullptr;
    PyObject*      distance_arg      = nullptr;
    int            number_of_threads = 0;
    int            precision         = 6; //should be default precision
    int            verbosity         = 0; //>0 to say what's happening
    const double*  distance_data     = nullptr;
    size_t         distance_entries  = 0;
    PyObject*      sequence_arg      = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, keywords, "sOO|iii", 
                                        const_cast<char**>(&argument_names[0]),
                                        &algorithm_name,  &sequence_arg,
                                        &distance_arg, &number_of_threads,
                                        &precision, &verbosity)) 
    {
        return nullptr;
    }        
    std::stringstream complaint;
    complaint << "Error: ";
    StartTree::BuilderInterface* algorithm = nullptr;
    if (algorithm_name==nullptr) {
        complaint << "Algorithm name not specified";
    } else {
        algorithm = StartTree::Factory::getInstance().getTreeBuilderByName(algorithm_name);
    }
    std::string  problem;
    std::string  tree_string;
    StrVector    sequences;
    DoubleVector local_distance_vector; //Not needed if distance_matrix is NumPy array
    if (algorithm==nullptr) {
        if (algorithm_name!=nullptr) {
            complaint << "Algorithm " << algorithm_name << " not found.";
        }
    } else {
        if (distance_arg==nullptr) {
            complaint << "No distances were supplied";
        }
        else if (!isVectorOfString("sequencenames", sequence_arg,
                                   sequences, complaint)) {
        }
        else if (sequences.size()<3) {
            complaint << "sequencenames contains only " << sequences.size()
                      << " sequences (must have at least 3).";
        }
        else if (isMatrix(distance_arg) && !isMatrixOfDouble("distance", distance_arg,
                                                 distance_data, distance_entries, complaint)) {
            //Will have set complaint
            //Problem. What if lower precision type is wanted?
        }
        else if (!isVectorOfDouble("distance", distance_arg, local_distance_vector,
                                   distance_data, distance_entries, complaint )) {
            //Will have set complaint
            //To think about later.  Might caller want to request
            //single precision?  Because that uses about half as much
            //memory and is ~ 20% *faster* (if a bit less accurate).
        }
        else if (distance_entries != sequences.size() * sequences.size()) {
            complaint << "There are " << sequences.size() << " sequences"
                      << " but the distance matrix"
                      << " contains " << distance_entries << " elements"
                      << " (should be " << (sequences.size() * sequences.size()) << ").";
        }
        else if (!obeyThreadCount(number_of_threads, complaint)) {
            //Will have set complaint
        }
        else if (precision<1)
            complaint << "Cannot have precision (" << precision << ")"
                      << " less than 1";
        else {
            if (verbosity==0) {
                algorithm->beSilent();
            }
            if (1<verbosity) {
                std::cout << "Sequences: " << sequences.join(",") << "\n";
                std::cout << "First two distances: " 
                          << distance_data[0] << " and "
                          << distance_data[1] << ".\n";
            }
            if (!algorithm->constructTreeStringInMemory
                 ( sequences, distance_data, tree_string )) {
                complaint << "Call to constructTreeStringInMemory failed "
                          << " for algorithm " << algorithm_name << ".";
                tree_string.clear();
            }
        }
    }
    if (tree_string.empty()) {
        PyErr_SetString(PyExc_TypeError, complaint.str().c_str());
        return nullptr;
    } else {
        PyObject* tree_result = StringToPythonString(tree_string);
        return tree_result;
    }
}

PyObject* StringVectorToPythonList(const StrVector& string_vector) {
    PyObject* list = PyList_New(string_vector.size());
    for (size_t i=0; i < string_vector.size() ; ++i ) {
        const std::string& name = string_vector[i];
        PyObject* item = StringToPythonString(name);
        if (item==nullptr) {
            continue;
        }
        if (PyList_SetItem(list,i,item) != 0) {
            Py_DECREF(item);
            Py_DECREF(list);
            return nullptr;
        }
    }
    return list;
}

static PyObject* pydecenttree_getAlgorithmNames
    ( PyObject* self, PyObject* args, PyObject* keywords ) {
    const char* argument_names[] = {
        "descriptions", nullptr
    };
    int descriptions = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywords, "|i", 
                                     const_cast<char**>(&argument_names[0]), 
                                     &descriptions)) {
        return nullptr;
    }
    bool withDescriptions = (descriptions!=0);
    StartTree::Factory& factory = StartTree::Factory::getInstance();
    auto names   = factory.getVectorOfTreeBuilderNames(withDescriptions);
    return StringVectorToPythonList(names);
}

static PyMethodDef pydecenttree_methods[3] = {
    { "constructTree", (PyCFunction) pydecenttree_constructTree,
      METH_VARARGS | METH_KEYWORDS, "Construct tree" },
    { "getAlgorithmNames", (PyCFunction) pydecenttree_getAlgorithmNames,
      METH_VARARGS | METH_KEYWORDS, 
      "Return a list of available algorithms" },
    { NULL, NULL, 0, NULL },
};

static PyModuleDef pydecenttree = {
    PyModuleDef_HEAD_INIT,
    "pydecenttree",
    "", /* module doco */
    -1, /* no per-instance state */
    pydecenttree_methods
};

extern "C" {
    PyMODINIT_FUNC PyInit_pydecenttree(void) {
        import_array();
        return PyModule_Create(&pydecenttree);
    }
};