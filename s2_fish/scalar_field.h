/*******************************************
** scalar_field.h
** Contains:
** 
*******************************************/

#ifndef HAVE_SCALAR_FIELD_H
#define HAVE_SCALAR_FIELD_H

#include "Double_tensor.h"
#include "TripleT.h"
#include <iostream>

class Scalar_Field
{
 private:
  Double_tensor points;
  double3 physical_size;
  double3 physical_offset;
  int3 num_points;

  double3 left;
  double3 h;
  
  void update_constants( );

 public:
  Scalar_Field(  );
  Scalar_Field(int3 num_points, double3 physical_size, double3 physical_offset );
  ~Scalar_Field();

  void set_num_points( int3 num_points );
  int3 get_num_points();
  void set_physical_params( double3 physical_size, double3 physical_offset );
  double3 get_physical_size();
  double3 get_physical_offset();
  void zero_the_points( );
  void set_point( int3 indices, double val );
  double get_point( int3 indices );
  void add_to_point( int3 indices, double val );
  int3 get_leftmost_indices( double3 location );
  double3 get_leftmost_offsets( double3 location );
  void print_points();
  int array_length();
  double* array_base_address();
};

#endif				//	ifndef HAVE_SCALAR_FIELD_H

