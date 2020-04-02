#ifndef CONTAINER_CONVERSIONS_H_
#define CONTAINER_CONVERSIONS_H_

#include <boost/python/extract.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/tuple.hpp>

namespace container_conversions {
  namespace bp = boost::python;

  template <typename CT>
  struct to_tuple {
    static bp::tuple convert_tuple(CT const& a) {
      bp::list result;
      typedef typename CT::const_iterator const_iter;
      for (const_iter p = a.begin(); p != a.end(); p++) {
        result.append(bp::object(*p));
      }
      return bp::tuple(result);
    }
    static PyObject* convert(CT const& a) {
      return bp::incref(convert_tuple(a).ptr());
    }
  };

  struct default_policy {
    static bool check_convertibility_per_element() { return false; }

    template <typename CT>
    static bool check_size(boost::type<CT>, std::size_t /*sz*/) {
      return true;
    }

    template <typename CT>
    static void assert_size(boost::type<CT>, std::size_t /*sz*/) {}

    template <typename CT>
    static void reserve(CT& a, std::size_t sz) {}
  };

  struct variable_capacity_policy : default_policy {
    template <typename CT>
    static void reserve(CT& a, std::size_t sz) {
      a.reserve(sz);
    }

    template <typename CT, typename ValueType>
    static void set_value(CT& a,
                          std::size_t
#if !defined(NDEBUG)
                              i
#endif
                          ,
                          ValueType const& v) {
      assert(a.size() == i);
      a.push_back(v);
    }
  };

  bool is_python_iterable(PyObject* p) {
     return (PyList_Check(p) || PyTuple_Check(p) || PyIter_Check(p));
  }

  bool metaclass_is_boost_python_class(PyObject* p) {
    //if (p->ob_type == 0) return false;
    PyTypeObject* type = Py_TYPE(p);
    if (type == 0) return false;
#if PY_MAJOR_VERSION >= 3
    auto p_base = type->ob_base.ob_base.ob_type;
#else
    auto p_base = type->ob_type;
#endif
    if (p_base == 0) return false;
    if (p_base->tp_name == 0) return false;
    return std::strcmp(p_base->tp_name, "Boost.Python.class") == 0;
  };

  template <typename CT, typename ConversionPolicy>
  struct from_python_sequence {
    typedef typename CT::value_type container_element_type;

    from_python_sequence() {
      bp::converter::registry::push_back(&convertible, &construct,
                                         bp::type_id<CT>());
    }

    static void* convertible(PyObject* p) {
        if (!(is_python_iterable(p) ||
              (!PyBytes_Check(p) &&
               !PyUnicode_Check(p) &&
               !metaclass_is_boost_python_class(p) &&
               PyObject_HasAttrString(p, "__len__") &&
               PyObject_HasAttrString(p, "__getitem__")
              )
             )
           )
        return 0;
      bp::handle<> obj_iter(bp::allow_null(PyObject_GetIter(p)));
      if (!obj_iter.get()) {  // must be convertible to an iterator
        PyErr_Clear();
        return 0;
      }
      if (ConversionPolicy::check_convertibility_per_element()) {
        int obj_size = PyObject_Length(p);
        if (obj_size < 0) {  // must be a measurable sequence
          PyErr_Clear();
          return 0;
        }
        if (!ConversionPolicy::check_size(boost::type<CT>(), obj_size))
          return 0;
        bool is_range = PyRange_Check(p);
        int i = 0;
        if (!all_elements_convertible(obj_iter, is_range, i)) return 0;
        if (!is_range) assert(i == obj_size);
      }
      return p;
    }

    // This loop factored out by Achim Domma to avoid Visual C++
    // Internal Compiler Error.
    static bool all_elements_convertible(bp::handle<>& obj_iter, bool is_range,
                                         int& i) {
      for (;; i++) {
        bp::handle<> py_elem_hdl(bp::allow_null(PyIter_Next(obj_iter.get())));
        if (PyErr_Occurred()) {
          PyErr_Clear();
          return false;
        }
        if (!py_elem_hdl.get()) break;  // end of iteration
        bp::object py_elem_obj(py_elem_hdl);
        bp::extract<container_element_type> elem_proxy(py_elem_obj);
        if (!elem_proxy.check()) return false;
        if (is_range) break;  // in a range all elements are of the same type
      }
      return true;
    }

    static void construct(PyObject* obj_ptr,
                          bp::converter::rvalue_from_python_stage1_data* data) {
      bp::handle<> obj_iter(PyObject_GetIter(obj_ptr));
      void* storage =
          ((bp::converter::rvalue_from_python_storage<CT>*)data)->storage.bytes;
      new (storage) CT();
      data->convertible = storage;
      CT& result = *((CT*)storage);
      std::size_t i = 0;
      for (;; i++) {
        bp::handle<> py_elem_hdl(bp::allow_null(PyIter_Next(obj_iter.get())));
        if (PyErr_Occurred()) bp::throw_error_already_set();
        if (!py_elem_hdl.get()) break;  // end of iteration
        bp::object py_elem_obj(py_elem_hdl);
        bp::extract<container_element_type> elem_proxy(py_elem_obj);
        ConversionPolicy::set_value(result, i, elem_proxy());
      }
      ConversionPolicy::assert_size(boost::type<CT>(), i);
    }
  };

  template <class T1, class T2>
  struct PairToTupleConverter {
    static PyObject* convert(const std::pair<T1, T2>& pair) {
      return bp::incref(bp::make_tuple(pair.first, pair.second).ptr());
    }
  };
}  // namespace container_conversions

#endif /* CONTAINER_CONVERSIONS_H_ */
