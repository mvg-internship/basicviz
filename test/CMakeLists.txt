add_executable(benchstat
  bench.cpp)

target_link_libraries(benchstat
  PRIVATE
    Lorina
)
