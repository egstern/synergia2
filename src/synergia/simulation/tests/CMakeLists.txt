add_executable(test_bunch_simulator_mpi test_bunch_simulator_mpi.cc)
target_link_libraries(test_bunch_simulator_mpi synergia_simulation
                      synergia_test_main)

add_mpi_test(test_bunch_simulator_mpi 1)
add_mpi_test(test_bunch_simulator_mpi 2)
add_mpi_test(test_bunch_simulator_mpi 3)
add_mpi_test(test_bunch_simulator_mpi 4)
add_mpi_test(test_bunch_simulator_mpi 5)
add_mpi_test(test_bunch_simulator_mpi 6)

add_executable(test_bunch_simulator test_bunch_simulator.cc)
target_link_libraries(test_bunch_simulator synergia_simulation
                      synergia_test_main)
add_mpi_test(test_bunch_simulator 1)

if(BUILD_PYTHON_BINDINGS)
  add_py_test(test_propagator.py)
  add_py_test(test_nonlinear_maps.py)
  add_py_test(test_booster_normal_form.py)
  add_py_test(test_tune_circular_lattice.py)
  add_py_test(test_tune_circular_lattice2.py)
  add_py_test(test_prop_actions.py)
  add_py_test(test_accel.py)
  add_py_test(test_accel2.py)
  add_py_test(test_accel_booster.py)
endif()
