#include <lorina/bench.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace lorina;

struct bench_statistics
{
  uint32_t number_of_inputs = 0;
  uint32_t number_of_outputs = 0;
  uint32_t number_of_dffs = 0;

  /* lines without input and outputs */
  uint32_t number_of_lines = 0;
};

class bench_statistics_reader : public bench_reader
{
public:
  explicit bench_statistics_reader( bench_statistics& stats )
      : _stats( stats )
  {
  }

  virtual void on_input( const std::string& name ) const override
  {
    (void)name;
    ++_stats.number_of_inputs;
  }

  virtual void on_output( const std::string& name ) const override
  {
    (void)name;
    ++_stats.number_of_outputs;
  }

  virtual void on_dff_input( const std::string& input ) const override
  {
    (void)input;
  }

  virtual void on_dff( const std::string& input, const std::string& output ) const override
  {
    (void)input;
    (void)output;
    ++_stats.number_of_dffs;
  }

  virtual void on_gate( const std::vector<std::string>& inputs, const std::string& output, const std::string& type ) const override
  {
    gate_lines.emplace_back( inputs, output, type );
    ++_stats.number_of_lines;
  }

  virtual void on_assign( const std::string& input, const std::string& output ) const override
  {
    (void)input;
    (void)output;
    ++_stats.number_of_lines;
  }

  bench_statistics& _stats;
  mutable std::vector<std::tuple<std::vector<std::string>,std::string,std::string>> gate_lines;
}; /* bench_statistics_reader */

static void
dump_statistics(FILE *f, const bench_statistics &st) {
  fprintf(f, "inputs: %u, outputs: %u, num ddfs: %u, num lines: %u\n",
      st.number_of_inputs,
      st.number_of_outputs,
      st.number_of_dffs,
      st.number_of_lines);
}

int
main(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::ifstream ifs(argv[i]);

    bench_statistics stats;
    bench_statistics_reader reader(stats);
    auto result = read_bench(ifs, reader);
    if (result == return_code::success) {
      dump_statistics(stdout, stats);
    }
  }
  return 0;
}
