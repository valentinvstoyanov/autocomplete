#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "../include/autocomplete.h"
#include "../include/seeder.h"
#include "../include/command_parser.h"

void print_greeting() {
  std::cout << "Welcome to autocomplete console application!" << std::endl;
}

template<typename Char = char, typename Word = std::basic_string<Char>>
time_t seed_autocomplete(unsigned size, char** const filenames, Autocomplete<Char, Word>& autocomplete) {
  time_t start = time(NULL);

  const auto file_callback = [](const char* filename, bool success) -> bool {
	if (success)
	  std::cout << "Reading from " << filename << "...\n";
	else
	  std::cerr << "Failed to read from " << filename << "\n";

	return success;
  };

  size_t counter = 0;
  const auto word_callback = [&counter](const Word& word) -> bool {
	std::cout << '\t' << ++counter << ' ' << word << " inserted...\n";
	return true;
  };

  Seeder::seedFromFiles<Char, Word>(size, filenames, autocomplete, file_callback, word_callback);

  return time(NULL) - start;
}

template<typename Char = char, typename Word = std::basic_string<Char>>
void run_input_parser(Autocomplete<Char, Word>& autocomplete) {
  std::vector<Word> suggestions;
  Word prefix;
  do {
	std::cout << '>';
	std::cin >> prefix;

	if (prefix == ":q")
	  break;

	bool suggested = autocomplete.suggest(prefix, [&prefix](const Word& word) {
	  std::cout << '\t' << prefix << word << "\n";
	});

	if (!suggested)
	  std::cout << "Nothing to suggest for " << prefix << std::endl;
  } while (true);
}

int main(int argc, char** argv) {
  print_greeting();

  if (argc <= 1) {
	std::cout << "No words data set supplied. Run with some command line arguments" << std::endl;
	return 0;
  }

  using Char = char;
  using Word = std::basic_string<Char>;

  Autocomplete<Char, Word> autocomplete;

  time_t elapsed_time = seed_autocomplete(static_cast<unsigned int>(argc - 1), argv + 1, autocomplete);
  std::cout << "Seeding finished in " << elapsed_time << 's' << std::endl;
  std::cout << "Total words seeded " << autocomplete.wordCount() << std::endl;

  if (!autocomplete.empty())
  	run_input_parser(autocomplete);

  return 0;
}