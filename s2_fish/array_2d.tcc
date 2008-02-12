#include <iostream>

template<class T>
Array_2d<T>::Array_2d() : Array_nd<T>()
{
    this->shape = vector2(0,0);
    this->strides = vector2(0,0);
}

template<class T>
Array_2d<T>::Array_2d(const int nx, const int ny) : Array_nd<T>()
{
    this->construct(vector2(nx,ny),true);
}

template<class T>
Array_2d<T>::Array_2d(const int nx, const int ny, 
        const int stride_x, const int stride_y) : Array_nd<T>()
{
    this->construct(vector2(nx,ny),vector2(stride_x,stride_y),true);
}

template<class T>
Array_2d<T>::Array_2d(const int nx, const int ny, T *data_ptr) : Array_nd<T>()
{
    this->data_ptr = data_ptr;
    this->construct(vector2(nx,ny),false);
}

template<class T>
Array_2d<T>::Array_2d(const int nx, const int ny, 
        const int stride_x, const int stride_y, T *data_ptr) : Array_nd<T>()
{
    this->data_ptr = data_ptr;
    this->construct(vector2(nx,ny),vector2(stride_x,stride_y),false);
}

template<class T>
Array_2d<T>::Array_2d(const Array_nd<T>& original)
{
    //~ std::cout << "converting from nd to 2d\n";
    if (original.get_rank() != 2) {
        throw 
            std::runtime_error("Attempt to convert Array_nd of rank !=2 to Array_2d");
    }
    copy_construct(original);
}


template<class T>
void
Array_2d<T>::reshape(const int nx, const int ny)
{
    bool shape_changed = this->different_shape(vector2(nx,ny));
    if (shape_changed && this->shape_frozen) {
        throw
        std::out_of_range("Attempt to change the shape of a frozen Array_2d");
    }
    if (shape_changed) {
        this->construct(vector2(nx,ny),true);
    }
}

template<class T>
void
Array_2d<T>::reshape(const int nx, const int ny,
    const int stride_x, const int stride_y)
{
    bool shape_changed = this->different_shape(vector2(nx,ny));
    if (shape_changed && this->shape_frozen) {
        throw
        std::out_of_range("Attempt to change the shape of a frozen Array_2d");
    }
    if (shape_changed) {
        this->construct(vector2(nx,ny),vector2(stride_x,stride_y),true);
    }
}

template<class T>
void
Array_2d<T>::reshape(const int nx, const int ny, T *data_ptr)
{
    bool shape_changed = this->different_shape(vector2(nx,ny));
    if (shape_changed && this->shape_frozen) {
        throw
        std::out_of_range("Attempt to change the shape of a frozen Array_2d");
    }
    this->data_ptr = data_ptr;
    if (shape_changed) {
        this->construct(vector2(nx,ny),false);
    }
}

template<class T>
void
Array_2d<T>::reshape(const int nx, const int ny,
    const int stride_x, const int stride_y, T *data_ptr)
{
    bool shape_changed = this->different_shape(vector2(nx,ny));
    if (shape_changed && this->shape_frozen) {
        throw
        std::out_of_range("Attempt to change the shape of a frozen Array_2d");
    }
    this->data_ptr = data_ptr;
    if (shape_changed) {
        this->construct(vector2(nx,ny),vector2(stride_x,stride_y),false);
    }
}

//~ template<class T>
//~ typename Array_2d<T>::iterator
//~ Array_2d<T>::begin()
//~ {
    //~ return iterator(this->data_ptr,this->shape,this->strides);
//~ }

//~ template<class T>
//~ typename Array_2d<T>::iterator
//~ Array_2d<T>::end()
//~ {
    //~ return iterator(0,this->shape,this->strides);
//~ }

template<class T>
inline int
Array_2d<T>::offset(const int i, const int j) const
{
    return i*this->strides[0] + j*this->strides[1];
}

    
template<class T>
inline T&
Array_2d<T>::at(const int i, const int j)
{
    if (bounds_check(i,j)) {
        return this->data_ptr[offset(i,j)];
    } else {
        throw
        std::out_of_range("Bounds check failed");
    }
}

template<class T>
inline T
Array_2d<T>::at(const int i, const int j) const
{
    if (bounds_check(i,j)) {
        return this->data_ptr[offset(i,j)];
    } else {
        throw
        std::out_of_range("Bounds check failed");
    }
}

template<class T>
inline T&
Array_2d<T>::operator()(const int i, const int j)
{
    return this->data_ptr[offset(i,j)];
}

template<class T>
inline T
Array_2d<T>::operator()(const int i, const int j) const
{
    return this->data_ptr[offset(i,j)];
}

template<class T>
inline bool
Array_2d<T>::bounds_check(const int i, const int j) const
{
    if ((i<0) || (i>this->shape[0]) || (j<0) || (j>this->shape[1])) {
        return false;
    }
    return true;
}

template<class T>
Array_2d<T>::~Array_2d()
{
}
