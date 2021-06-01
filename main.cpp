#include <iostream>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <vector>
#include <pal.hpp>

constexpr size_t MAX_SOURCE_SIZE = 100000;
constexpr size_t DATA_SIZE = 10;

std::vector<GridNode> load_grid(const char* filename) {
  std::vector<GridNode> grid{};

  std::ifstream file;
  file.open(filename, std::ios::binary);
  if (!file.good()) {
    std::cout << "Grid file " << filename << " does not exist!" << std::endl;
    exit(1);
  }

  GridNode tmp;
  while (file) {
    file.read((char*)&tmp, sizeof(GridNode));
    if (file.gcount() == 0) {
      break;
    }
    grid.push_back(tmp);
  }
  file.close();
  return grid;
}

std::vector<Sample> load_samples(const char* filename) {
  std::vector<Sample> samples;

  std::ifstream file;
  file.open(filename);
  if (!file.good()) {
    std::cout << "File " << filename << " does not exist!" << std::endl;
    exit(1);
  }

  Sample tmp;
  while (file) {
    file >> tmp.multiplicity[0];
    file >> tmp.multiplicity[1];
    file >> tmp.multiplicity[2];
    file >> tmp.multiplicity[3];
    file >> tmp.multiplicity[4];
    if (file.eof()) { // Failsafe in case of new line at the end of file
      break;
    }
    file.get();
    samples.push_back(tmp);
  }
  file.close();
  std::cout << "Loaded: " << samples.size() << " samples\n";
  return samples;
}

int main(int argc, char** argv) {

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <samples_file> [output_file] [grid_file]" << std::endl;
    std::cout << "\tsamples_file: name of input file containg samples for estimation" << std::endl;
    std::cout << "\toutput_file: name of csv file containing results" << std::endl;
    std::cout << "\tgrid_file: name of file with psi, rho and probabilities on which to base the estimation" << std::endl;
    return 1;
  }

  const char* samples_filename = argv[1];
  const char* output_filename = "output.csv";
  const char* grid_filename = "gsd_prob_grid.bin";
  if (argc > 2) {
    output_filename = argv[2];
  }
  if (argc > 3) {
    grid_filename = argv[3];
  }

  auto samples = load_samples(samples_filename);
  auto grid = load_grid(grid_filename);
  std::vector<float> results{};
  std::vector<float> max_likelihood(samples.size(), -std::numeric_limits<float>::infinity());
  std::vector<float> max_likelihood_idx(samples.size());

  size_t remaining_samples = samples.size();
  size_t offset = 0;

  auto start = std::chrono::system_clock::now();

  Context pal_context;
  pal_context.init();
  pal_context.prepare_kernel();
  std::cerr << "Prepare Kernel Succeded!" << std::endl;

  while (remaining_samples > 0) {
    size_t loaded = pal_context.setup_buffers(samples.data()+offset, remaining_samples, grid.data(), grid.size());
    results.reserve(loaded*grid.size());

    std::cerr << "Loaded: " << loaded << " samples. " << remaining_samples-loaded << " remining." << std::endl;
    std::cerr << "Setup buffers Succeded!" << std::endl;

    pal_context.start();
    pal_context.join();
    std::cerr << "Kernel Succeded!" << std::endl;

    pal_context.read_data(results.data());

    for (size_t i = 0; i < grid.size(); ++i) {
      for (size_t n = 0; n < loaded; ++n) {
        if (results[loaded*i + n] > max_likelihood[n+offset]) {
          max_likelihood[n+offset] = results[loaded*i + n];
          max_likelihood_idx[n+offset] = i;
        }
      }
    }

    offset += loaded;
    remaining_samples -= loaded;
  }

  auto end = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "Whole process took: " << duration << "ms => AVG: " << duration / (float)samples.size() << "ms per sample" << std::endl;

  std::ofstream file;
  file.open(output_filename);

  std::cout << "Writing to file..." << std::endl;

  file << "idx,psi,rho,log_likelihood" << std::endl;
  for (size_t i = 0; i < samples.size(); ++i) {
    size_t idx = max_likelihood_idx[i];
    file << i << ',' << grid[idx].psi << ',' << grid[idx].rho << ',' << max_likelihood[i] << std::endl;
  }
  file.close();

  return 0;
}
