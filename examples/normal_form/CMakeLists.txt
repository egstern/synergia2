add_executable(normal_form normal_form.cc)
add_executable(bad_sbend_map bad_sbend_map.cc)
add_executable(ok_sbend_map ok_sbend_map.cc)

target_link_libraries(normal_form synergia_simulation synergia_lattice
                      synergia_bunch)
target_link_libraries(bad_sbend_map synergia_simulation synergia_lattice
                      synergia_bunch)
target_link_libraries(ok_sbend_map synergia_simulation synergia_lattice
                      synergia_bunch)

copy_file(channel.madx normal_form)
copy_file(normal_form.py normal_form)
copy_file(bad_sbend_map normal_form)
copy_file(ok_sbend_map normal_form)

add_executable(channel_map channel_map.cc)
target_link_libraries(channel_map synergia_lattice synergia_simulation)

copy_file(channel_map.py channel_map)
copy_file(foborodobo128.madx channel_map)

