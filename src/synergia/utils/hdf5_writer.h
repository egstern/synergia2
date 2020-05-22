#ifndef HDF5_WRITER_H_
#define HDF5_WRITER_H_
#include <vector>
#include <string>

#include "hdf5.h"

#include "synergia/utils/hdf5_misc.h"

template<typename T>
    class Hdf5_writer
    {
    private:
        int data_rank;
        std::vector<hsize_t > dims;
        std::string name;

        // use hid_t type since the writer doesnt own the file object
        hid_t file_ptr;    
        Hdf5_handler atomic_type;

        void
        write_storage_order(T const& data);
        void
        update_dims(T const& data);

        const void *
        get_data_ptr(T const& data);
    public:
        static const int c_storage_order = 0;
        static const int fortran_storage_order = 1;
        Hdf5_writer(hid_t file_ptr, std::string const& name);
        void
        write(T const & data);
        void
        write(T const * data, size_t len);
    };

#include "synergia/utils/hdf5_writer.tcc"
#endif /* HDF5_WRITER_H_ */
