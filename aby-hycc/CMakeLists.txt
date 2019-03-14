
link_directories(../../../../cbmc-gc/src/libcircuit)

add_executable(aby-hycc main.cpp hycc_adapter.cpp circuit_converter.cpp)

target_include_directories(aby-hycc PUBLIC
  ../../../../cbmc-gc/src/circuit-utils/include
  ../../../../cbmc-gc/src
  ../../../../cbmc-gc/src/cbmc/src
)

target_link_libraries(aby-hycc
  ABY::aby
  ENCRYPTO_utils::encrypto_utils
  circuit
)
