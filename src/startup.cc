#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "../include/autocomplete.h"
#include "../include/seeder.h"
#include "../include/command_parser.h"

using Char = wchar_t;
using Word = std::basic_string<Char>;

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
	std::wcout << L'\t' << ++counter << L' ' << word << L" inserted...\n";
	return true;
  };

  Seeder::seedFromFiles<Char, Word>(size, filenames, autocomplete, file_callback, word_callback);

  return time(NULL) - start;
}

void run_input_parser(Autocomplete<Char, Word>& autocomplete) {
  std::vector<Word> suggestions;
  Word prefix;
  do {
	std::wcout << L'>';
	std::wcin >> prefix;

	if (prefix == L":q")
	  break;

	bool suggested = autocomplete.suggest(prefix, [&prefix](const Word& word) {
	  std::wcout << L'\t' << prefix << word << "\n";
	});

	if (!suggested)
	  std::wcout << L"Nothing to suggest for " << prefix << std::endl;
  } while (true);
}

int main(int argc, char** argv) {
  if (argc <= 1) {
	std::cout << "No words data set supplied. Run with some command line arguments" << std::endl;
	return 0;
  }

  Autocomplete<Char, Word> autocomplete;

  time_t elapsed_time = seed_autocomplete(static_cast<unsigned int>(argc - 1), argv + 1, autocomplete);
  std::cout << "Seeding finished in " << elapsed_time << 's' << std::endl;
  std::cout << "Total words seeded " << autocomplete.wordCount() << std::endl;

  if (!autocomplete.empty())
	run_input_parser(autocomplete);

  return 0;
}