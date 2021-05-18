#include <iostream>
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>
#include <pal.hpp>

constexpr size_t MAX_SOURCE_SIZE = 100000;
constexpr size_t DATA_SIZE = 10;

std::vector<GridNode> load_grid() {
  std::vector<GridNode> grid{};

  std::ifstream file;
  file.open("gsd_prob_grid.bin", std::ios::binary);

  GridNode tmp;
  while(file) {
    file.read((char*)&tmp, GRIDNODE_SIZE);
    if( file.gcount() == 0 )
      break;
    grid.push_back(tmp);
  }
  file.close();
  return std::move(grid);
}

std::vector<Sample> load_samples() {
  std::vector<Sample> samples;

  std::ifstream file;
  file.open("samples.txt");

  Sample tmp;
  while( file ) {
    file >> tmp.multiplicity[0];
    file >> tmp.multiplicity[1];
    file >> tmp.multiplicity[2];
    file >> tmp.multiplicity[3];
    file >> tmp.multiplicity[4];
    file.get();
    samples.push_back(tmp);
  }
  file.close();
  std::cout << "Loaded: " << samples.size() << " samples\n";
  return std::move(samples);
}

int main() {

  auto grid = load_grid();
  auto samples = load_samples();
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

    for  (int i = 0; i < grid.size(); ++i) {
      for (int n = 0; n < loaded; ++n) {
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
  file.open("output.csv");

  std::cout << "Writing to file..." << std::endl;

  file << "idx,psi,rho,log_likelihood" << std::endl;
  for( int i = 0; i < samples.size(); ++i ) {
    size_t idx = max_likelihood_idx[i];
    file << i << ',' << grid[idx].psi << ',' << grid[idx].rho << ',' << max_likelihood[i] << std::endl;
  }
  file.close();

  return 0;
}
