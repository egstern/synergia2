#ifndef NUMPY_MULTI_ARRAY_CONVERTER_H_
#define NUMPY_MULTI_ARRAY_CONVERTER_H_

#include "numpy/arrayobject.h"

template<typename ValueType, int Dimension>
    struct numpy_multi_array_converter
    {
        typedef boost::multi_array<ValueType, Dimension > multi_array_t;
        typedef std::vector<std::size_t > shape_t;

        static void
        register_to_and_from_python()
        {
            register_from_python();
            register_to_python();
        }

        static void
        register_to_python()
        {
            boost::python::to_python_converter<multi_array_t,
                    numpy_multi_array_converter<ValueType, Dimension > >();
        }

        static void
        register_from_python()
        {
            boost::python::converter::registry::push_back(
                    &numpy_multi_array_converter<ValueType, Dimension >::convertible,
                    &numpy_multi_array_converter<ValueType, Dimension >::construct,
                    boost::python::type_id<multi_array_t >());
        }

        static
        void *
        convertible(PyObject * obj)
        {
            using namespace boost::python;
            try {
                shape_t shape;
                get_shape(object(handle< >(borrowed(obj))), shape);
                if (multi_array_t::dimensionality != shape.size()) return 0;
            }
            catch (...) {
                return 0;
            }
            return obj;
        }

        static
        void
        construct(PyObject* obj,
                boost::python::converter::rvalue_from_python_stage1_data* data)
        {
            throw std::runtime_error("jfa: no worky!");
            using namespace boost::python;

            //get the storage
            typedef converter::rvalue_from_python_storage<multi_array_t > storage_t;
            storage_t * the_storage = reinterpret_cast<storage_t * >(data);
            void * memory_chunk = the_storage->storage.bytes;

            //new placement
            object py_obj(handle< >(borrowed(obj)));
            shape_t shape;
            get_shape(py_obj, shape);
            new (memory_chunk) multi_array_t(shape);
            data->convertible = memory_chunk;
        }

        static PyObject *
        convert(const multi_array_t & c_array)
        {
            using namespace boost::python;
            PyObject *retval;
            int type_num;
            if (typeid(ValueType) == typeid(double)) type_num = NPY_DOUBLE;
            else {
                throw std::runtime_error(
                        "numpy_multi_array_converter: Unable to deal with type");
            }
            retval = PyArray_SimpleNew(c_array.num_dimensions(),
                    (npy_intp*) c_array.shape(), type_num);
            double * p = (double *) PyArray_DATA(retval);
            for (const double * it = c_array.data();
                    it != (c_array.data() + c_array.num_elements()); ++it) {
                *p = *it;
                ++p;
            }
            return retval;
        }

    protected:
        static
        void
        get_shape(boost::python::object obj, shape_t & shape)
        {
            using namespace boost::python;
            shape.clear();
            object py_shape = obj.attr("shape");
            const std::size_t N = len(py_shape);
            for (std::size_t i = 0; N != i; ++i)
                shape.push_back(extract<std::size_t >(py_shape[i]));
        }
    };

#endif /* NUMPY_MULTI_ARRAY_CONVERTER_H_ */
