add_executable(test_bunch_traits test_bunch_traits.cc)
target_link_libraries(test_bunch_traits ${kokkos_libs})
add_mpi_test(test_bunch_traits 1)

add_executable(test_bunch test_bunch.cc)
target_link_libraries(test_bunch synergia_bunch synergia_test_main)
add_mpi_test(test_bunch 1)

add_executable(test_bunch_particles test_bunch_particles.cc)
target_link_libraries(test_bunch_particles synergia_bunch synergia_test_main)
add_mpi_test(test_bunch_particles 1)

add_py_test(test_bunch_reference_particles.py)
