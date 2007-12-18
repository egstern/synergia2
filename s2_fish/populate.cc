#include <iostream>

#include <iomanip>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_qrng.h>
#include <gsl/gsl_randist.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#include "populate.h"

class GSL_random
{
    private:
    gsl_rng * rng;
    public:
    GSL_random(unsigned long int seed = 0,
        const gsl_rng_type *generator = gsl_rng_ranlxd2);
    void fill_array_unit_gaussian(double *array, int num, int stride=1);
    void fill_array_uniform(double *array, int num, int stride=1);
    ~GSL_random();
};

GSL_random::GSL_random(unsigned long int seed,
        const gsl_rng_type *generator)
{
    gsl_rng_env_setup();
    rng = gsl_rng_alloc(generator);
    gsl_rng_set(rng,seed);
}

void
GSL_random::fill_array_unit_gaussian(double *array, int num, int stride)
{
    for(int i = 0; i<num; i+=stride) {
            array[i] = gsl_ran_ugaussian_ratio_method(rng);
    }
}

void
GSL_random::fill_array_uniform(double *array, int num, int stride)
{
    for(int i = 0; i<num; i+=stride) {
            array[i] = gsl_rng_uniform(rng);
    }
}

GSL_random::~GSL_random()
{
    gsl_rng_free(rng);
}

gsl_matrix
gsl_matrix_from_Array_2d(const Array_2d<double> &array)
{
    gsl_matrix matrix;
    std::vector<int> shape = array.get_shape();
    matrix.size1= shape.at(0);
    matrix.size2 = shape.at(1);
    matrix.tda = shape.at(1);
    matrix.data = array.get_data_ptr();
    matrix.block = 0;
    matrix.owner = 0;
    
    return matrix;
}

Array_2d<double>
Array_2d_from_gsl_matrix(const gsl_matrix &matrix)
{
    return Array_2d<double>(matrix.size1,matrix.size2,matrix.data);
}

void
array_2d_to_octave_file(const Array_2d<double> &array, const std::string filename)
{
    std::ofstream stream(filename.c_str());
    for (int i=0; i<array.get_shape()[0]; ++i) {
        for (int j=0; j<array.get_shape()[1]; ++j) {
            stream << std::setprecision(16) << array(i,j);
            if (j == array.get_shape()[1]-1) {
                stream << std::endl;
            } else {
                stream << " ";
            }
        }
    }
    stream.close();
}

void
get_means_covariances(const Array_2d<double> &array,
    Array_1d<double> &means,
    Array_2d<double> &covariances)
{
    int num = array.get_shape()[0];
    means.set_all(0.0);
    covariances.set_all(0.0);
    for(int n=0; n<num; ++n) {
        for(int j=0; j<6; ++j) means(j) += array(n,j);
    }
    means.scale(1.0/num);
    for(int n=0; n<num; ++n) {
        for (int i=0; i<6; ++i) {
            for (int j=0; j<=i; ++j) {
                covariances(i,j) += (array(n,i)-means(i))*(array(n,j)-means(j));
            }
        }
    }
    for (int i=0; i<6; ++i) {
        for (int j=i; j<6; ++j) {
            covariances(i,j) = covariances(j,i);
        }
    }
    covariances.scale(1.0/num);
}

void
adjust_moments(Array_2d<double> &array_in,
    Array_2d<double> &array_out,
    const Array_1d<double> &means,
    const Array_2d<double> &covariances)
{
    // Verify that covariance matrix is symmetric
    for (int i=0; i<6; ++i) {
        for (int j=i+1; j<6; ++j) {
            if (covariances.at(i,j) != covariances.at(j,i)) {
                throw
                    std::runtime_error("covariance matrix in adjust_moments is not symmetric");
            }
        }
    }
    
    Array_1d<double> actual_means(6);
    Array_2d<double> actual_covs(6,6);
    get_means_covariances(array_in,actual_means,actual_covs);
    
    // Calculate G
    gsl_matrix C = gsl_matrix_from_Array_2d(covariances);
    covariances.print("debug covariances");
    int err =  gsl_linalg_cholesky_decomp(&C);
    if (err !=0) {
        throw
            std::runtime_error("gsl_linalg_cholesky_decomp failed for matrix C");
    }
    gsl_matrix *G_ptr = &C;
    
    // Calculate H
    // actual_covs is const and cholesky overwrites, so work on a copy
    gsl_matrix X_tmp = gsl_matrix_from_Array_2d(actual_covs);
    gsl_matrix *X_ptr = gsl_matrix_alloc(6,6);
    gsl_matrix_memcpy(X_ptr,&X_tmp);
    Array_2d_from_gsl_matrix(X_tmp).print("copied matrix");
    // X -> H
    err =  gsl_linalg_cholesky_decomp(X_ptr);
    if (err !=0) {
        throw
            std::runtime_error("gsl_linalg_cholesky_decomp failed for matrix X");
    }
    gsl_matrix *H_ptr = X_ptr;
    
    // H_ptr and G_ptr are currently G + transp(G) and H + transp(H), so
    // we need to zero out the transp() parts
    for (int i=0; i<6; ++i) {
        for (int j = i+1; j<6; ++j) {
            gsl_matrix_set(H_ptr,i,j,0.0);
            gsl_matrix_set(G_ptr,i,j,0.0);
        }
    }
    
    // Calculate H^-1
    gsl_permutation *perm = gsl_permutation_alloc(6);
    int signum;
    err = gsl_linalg_LU_decomp(H_ptr, perm, &signum);
    if (err !=0) {
        throw
            std::runtime_error("gsl_linalg_LU_decomp  failed for matrix H");
    }
    gsl_matrix *Hinv_ptr = gsl_matrix_alloc(6,6);
    err = gsl_linalg_LU_invert(H_ptr, perm, Hinv_ptr);
    if (err !=0) {
        throw
            std::runtime_error("gsl_linalg_LU_invert  failed for matrix H");
    }
    
    // Calculate A
    gsl_matrix *A = gsl_matrix_alloc(6,6);
    gsl_blas_dgemm (CblasNoTrans, CblasNoTrans,1.0, G_ptr, Hinv_ptr, 0.0, A);
    
    // Subtract actual means
    for(int n=0; n<array_in.get_shape()[0]; ++n) {
        for(int j=0; j<6; ++j) array_in(n,j) -= actual_means(j);
    }
    
    gsl_matrix array_in_gsl = gsl_matrix_from_Array_2d(array_in);
    gsl_matrix array_out_gsl = gsl_matrix_from_Array_2d(array_out);
    
    gsl_blas_dgemm(CblasNoTrans, CblasTrans,1.0,&array_in_gsl,A,0.0,&array_out_gsl);
    
    gsl_matrix_free(X_ptr);
    gsl_permutation_free(perm);
    gsl_matrix_free(Hinv_ptr);
    gsl_matrix_free(A);    
}

void
populate_6d_gaussian(Array_2d<double> &particles, 
    const Array_1d<double> &means, const Array_2d<double> &covariances,
    const int id_offset)
{
    if (particles.get_shape()[1] != 7) {
        throw
        std::runtime_error("populate_6d_gaussian expects a particle array with shape (num_particles,7)");
    }
    int num_particles = particles.get_shape()[0];
    // Use the memory allocated for particles as a scratch area until the very end
    Array_2d<double> tmp(num_particles,6,particles.get_data_ptr());
    Array_2d<double> tmp2(num_particles,6);

    GSL_random gslr;
    gslr.fill_array_unit_gaussian(tmp.get_data_ptr(),tmp.get_size());
 
    adjust_moments(tmp,tmp2,means,covariances);
    
    // Fill output array, adding (requested) means and setting particle ID.
    for(int n=0; n<num_particles; ++n) {
        for(int j=0; j<6; ++j) particles(n,j) = tmp2(n,j) + means(j);
        particles(n,6) = (n + id_offset)*1.0;
    }
    array_2d_to_octave_file(particles,"debug.dat");
}
