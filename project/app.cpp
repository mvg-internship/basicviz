#include <lorina/bench.hpp>
#include <map>
#include <string>
#include <vector>

#include <cstdio>

#include <SDL.h>
#include <SDL_ttf.h>

enum {
  INPUT_WIDTH = 60,
  INPUT_HEIGHT = 45,
  SCREEN_WIDTH = 1600,
  SCREEN_HEIGHT = 900,
  GAPS = 80
};

struct parameters{
    std::string type;

    std::vector<std::string> gates;

    std::string name;

    int x;
    int y;

    bool setting_coord = false;
    bool set_coord = false;
};
struct bench_statistics {
  uint32_t number_of_inputs = 0;
  uint32_t number_of_outputs = 0;
  uint32_t number_of_dffs = 0;

  std::vector<std::string> inputs;

  std::vector<parameters> el;

  std::map<std::string, int> gatesMap;

  /* lines without input and outputs */
  uint32_t number_of_lines = 0;
};

void setMapToGates(bench_statistics &stats) {
  for (size_t i = 0; i < stats.el.size(); ++i)
      stats.gatesMap.insert(std::make_pair(stats.el.at(i).name,i));
}

class bench_statistics_reader : public lorina::bench_reader {
public:
  explicit bench_statistics_reader(bench_statistics &stats) : _stats(stats) {}

  virtual void on_input(const std::string &name) const override {
    parameters local;
    const std::string type = "INPUT";
    local.type = type;
    local.name = name;
    _stats.el.push_back(local);
    _stats.inputs.push_back(name);
    ++_stats.number_of_inputs;
  }

  virtual void on_output(const std::string &name) const override {
    ++_stats.number_of_outputs;
  }

  virtual void on_dff(const std::string &input,
                      const std::string &output) const override {
    std::string type = "DFF";
    parameters local;
    local.type = type;
    local.gates.push_back(input);
    local.name = output;
    _stats.el.push_back(local);
    ++_stats.number_of_dffs;
  }

  virtual void on_gate(const std::vector<std::string> &inputs,
                       const std::string &output,
                       const std::string &type) const override {
    gate_lines.emplace_back(inputs, output, type);
    parameters local;
    local.type = type;
    local.name = output;
    for (size_t i = 0; i < inputs.size(); ++i)
        local.gates.push_back(inputs[i]);
    _stats.el.push_back(local);
    _stats.number_of_lines++;
  }

  virtual void on_assign(const std::string &input,
                         const std::string &output) const override {
    ++_stats.number_of_lines;
  }

  bench_statistics &_stats;
  mutable std::vector<
      std::tuple<std::vector<std::string>, std::string, std::string>>
      gate_lines;
};

static void dump_statistics(FILE *f, const bench_statistics &st) {
  fprintf(f, "inputs: %u, outputs: %u, num ddfs: %u, num lines: %u\n",
          st.number_of_inputs, st.number_of_outputs, st.number_of_dffs,
          st.number_of_lines);
}

void setElCoord(bench_statistics &stats, int index_number) {
  // function that specifies coordinate for all elements
  int maxXCoord = 0;

  parameters &data = stats.el[index_number];
  if (data.set_coord)
    return;

  data.setting_coord = true;

  for (size_t i = 0; i < data.gates.size(); ++i) {
    if (data.set_coord)
      break;

    std::string gate = data.gates.at(i);
    int index = stats.gatesMap[gate];

    if (stats.el.at(index).set_coord) {
      maxXCoord = fmax(maxXCoord, stats.el.at(index).x);
    } else {
      if (stats.el.at(index).setting_coord) {
        stats.el.at(index).setting_coord = false;
        continue;
      }
      setElCoord(stats, index);

      maxXCoord = fmax(maxXCoord, stats.el.at(index).x);
    }
  }
  if (data.setting_coord)
    data.setting_coord =false;
  int y_coord = 0;
  for (size_t j = 0; j < stats.el.size(); ++j) {
    if (stats.el.at(j).x == maxXCoord + GAPS)
      y_coord = fmax(y_coord, stats.el.at(j).y + GAPS);
  }
  data.x = maxXCoord + 80;
  data.y = y_coord;
  data.set_coord =true;
}

void setInputsCoord(std::vector<parameters> &el, std::string name,
                    int index_number) {
  //    function that specifies coordinate for inputs
  for (size_t i = 0; i < el.size(); ++i)
    if (el.at(i).name == name) {
      el.at(i).x = 0;
      el.at(i).y = (INPUT_HEIGHT+ GAPS)  * index_number;
      el.at(i).set_coord =true;
    }
}

SDL_Window *sdlInitialization() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL2 could not be initialized!\n"
           "SDL2 Error: %s\n",
           SDL_GetError());

    return 0;
  }

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
  // Disable compositor bypass
  if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
    printf("SDL can not disable compositor bypass!\n");
    return 0;
  }
#endif
  SDL_Window *window = SDL_CreateWindow(
      "SDL2_ttf sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    printf("Window could not be created!\n"
           "SDL_Error: %s\n",
           SDL_GetError());
  }
  return window;
}

void eventLoop(bench_statistics &stats, SDL_Renderer *renderer,
               std::vector<SDL_Rect> &elements) {
  // Event loop exit flag
  bool quit = false;

  // Event loop
  while (!quit) {
    SDL_Event e;

    // Wait indefinitely for the next available event
    SDL_WaitEvent(&e);

    // User requests quit
    if (e.type == SDL_QUIT) {
      quit = true;
    }

    // Initialize renderer color white for the background
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // Clear screen
    SDL_RenderClear(renderer);

    // Set renderer color red to draw the square
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);

    // Draw filled square
    const std::vector<parameters> &el = stats.el;
    for (size_t i = 0; i < el.size(); ++i) {
        SDL_RenderFillRect(renderer,&elements[i]);
        for (size_t j = 0; j < el.at(i).gates.size(); ++j) {
          SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
          int el2 = stats.gatesMap[el.at(i).gates.at(j)];
          SDL_RenderDrawLine(renderer, el.at(i).x, el.at(i).y,
                  el.at(el2).x + INPUT_WIDTH,
                  el.at(el2).y + INPUT_HEIGHT);
        }
    }

    // Update screen
    SDL_RenderPresent(renderer);
  }
}

void renderElements(bench_statistics &stats, SDL_Renderer *renderer){
    //            set inputs coordinates
    for (size_t i = 0; i < stats.inputs.size(); ++i) {
        setInputsCoord(stats.el, stats.inputs.at(i), i);
    }

    //            set coordinates for other elements
    setMapToGates(stats);
    for (size_t i = 0; i < stats.el.size(); ++i) {
        setElCoord(stats, i);
    }

    //          Declare rect of square
    std::vector<SDL_Rect> elements(stats.el.size());

    for (size_t i = 0; i < stats.el.size(); ++i) {
        elements[i].w = INPUT_WIDTH;
        elements[i].h = INPUT_HEIGHT;
        elements[i].x = stats.el.at(i).x;
        elements[i].y = stats.el.at(i).y;
    }
    eventLoop(stats, renderer, elements);
}

void creteRenderer(bench_statistics &stats, SDL_Window *window) {
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    printf("Renderer could not be created!\n"
           "SDL_Error: %s\n",
           SDL_GetError());
  else{
      renderElements(stats,renderer);
      SDL_DestroyRenderer(renderer);
  }

}

int main(int argc, char *argv[]) {
  //    window initialization. This item belongs to main
  SDL_Window *window = sdlInitialization();

  for (int i = 1; i < argc; ++i) {
    std::ifstream ifs(argv[i]);

    bench_statistics stats;
    bench_statistics_reader reader(stats);
    auto result = read_bench(ifs, reader);

    if (result == lorina::return_code::success) {
      dump_statistics(stdout, stats);
    }

    creteRenderer(stats, window);

    SDL_DestroyWindow(window);

    TTF_Quit();

    SDL_Quit();
  }
  return 0;
}
