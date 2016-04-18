#include "hdf5_chunked_array2d_writer.h"
#include "hdf5_misc.h"
#include <stdexcept>
#include <iostream>

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

Hdf5_chunked_array2d_writer::Hdf5_chunked_array2d_writer(H5File * file_ptr,
        std::string const& name, Const_MArray2d_view const & initial_data,
        int chunk_size) :
        dims(2), max_dims(2), size(2), offset(2), chunk_dims(2),
        atomic_type(hdf5_atomic_data_type<double >())
{
    for (int i = 0; i < 2; ++i) {
        dims[i] = initial_data.shape()[i];
        max_dims[i] = initial_data.shape()[i];
        chunk_dims[i] = initial_data.shape()[i];
        offset[i] = 0;
    }
    if (chunk_size > 0) {
        chunk_dims[0] = chunk_size;
    }
    size[0] = 0;
    size[1] = initial_data.shape()[1];
    max_dims[0] = H5S_UNLIMITED;
    DSetCreatPropList cparms;
    cparms.setChunk(2, &chunk_dims[0]);
    DataSpace dataspace(2, &dims[0], &max_dims[0]);
    dataset = file_ptr->createDataSet(name.c_str(), atomic_type, dataspace,
            cparms);
}

Hdf5_chunked_array2d_writer::Hdf5_chunked_array2d_writer(H5File * file_ptr,
        std::string const& name, Const_MArray2d_ref const & initial_data,
        int chunk_size) :
        dims(2), max_dims(2), size(2), offset(2), chunk_dims(2),
        atomic_type(hdf5_atomic_data_type<double >())
{
    for (int i = 0; i < 2; ++i) {
        dims[i] = initial_data.shape()[i];
        max_dims[i] = initial_data.shape()[i];
        chunk_dims[i] = initial_data.shape()[i];
        offset[i] = 0;
    }
    if (chunk_size > 0) {
        chunk_dims[0] = chunk_size;
    }
    size[0] = 0;
    size[1] = initial_data.shape()[1];
    max_dims[0] = H5S_UNLIMITED;
    DSetCreatPropList cparms;
    cparms.setChunk(2, &chunk_dims[0]);
    DataSpace dataspace(2, &dims[0], &max_dims[0]);
    dataset = file_ptr->createDataSet(name.c_str(), atomic_type, dataspace,
            cparms);
}

void
Hdf5_chunked_array2d_writer::write_chunk(Const_MArray2d_ref const & data)
{
    chunk_dims[0] = data.shape()[0];
    chunk_dims[1] = data.shape()[1];
    DSetCreatPropList cparms;
    cparms.setChunk(2, &chunk_dims[0]);
    size[0] += data.shape()[0];
    dataset.extend(&size[0]);
    DataSpace filespace = dataset.getSpace();
    filespace.selectHyperslab(H5S_SELECT_SET, &chunk_dims[0], &offset[0]);
    DataSpace dataspace(2, &chunk_dims[0], &max_dims[0]);

    int nx = data.shape()[0];
    int ny = data.shape()[1];
    int x0 = data.index_bases()[0];
    int y0 = data.index_bases()[1];

    std::vector<double> temp(nx * ny);

    for (int i = x0; i < x0 + nx - 1; ++i)
    {
        for (int j = y0; j < y0 + ny - 1; ++j)
        {
            temp[i*ny + j] = data[i][j];
        }
    }

    dataset.write(&temp[0], atomic_type, dataspace, filespace);

    offset[0] += data.shape()[0];
}

void
Hdf5_chunked_array2d_writer::write_chunk(Const_MArray2d_view const & data)
{
    chunk_dims[0] = data.shape()[0];
    chunk_dims[1] = data.shape()[1];
    DSetCreatPropList cparms;
    cparms.setChunk(2, &chunk_dims[0]);
    size[0] += data.shape()[0];
    dataset.extend(&size[0]);
    DataSpace filespace = dataset.getSpace();
    filespace.selectHyperslab(H5S_SELECT_SET, &chunk_dims[0], &offset[0]);
    DataSpace dataspace(2, &chunk_dims[0], &max_dims[0]);

    int nx = data.shape()[0];
    int ny = data.shape()[1];
    int x0 = data.index_bases()[0];
    int y0 = data.index_bases()[1];

    std::vector<double> temp(nx * ny);

    for (int i = x0; i < x0 + nx - 1; ++i)
    {
        for (int j = y0; j < y0 + ny - 1; ++j)
        {
            temp[i*ny + j] = data[i][j];
        }
    }

    dataset.write(&temp[0], atomic_type, dataspace, filespace);

    offset[0] += data.shape()[0];
}

Hdf5_chunked_array2d_writer::~Hdf5_chunked_array2d_writer()
{
}
