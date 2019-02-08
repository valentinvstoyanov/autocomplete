#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "../include/autocomplete.h"
#include "../include/seeder.h"

using Char = wchar_t;
using Word = std::basic_string<Char>;

time_t seed_autocomplete(unsigned size, char** const filenames, Autocomplete<Char, Word>& autocomplete) {
  time_t start = time(NULL);

  const auto file_callback = [](const char* filename, bool success) -> bool {
	/*if (success)
	  std::wcout << "Reading from " << filename << "...\n";
	else
	  std::cerr << "Failed to read from " << filename << "\n";
	*/
	return success;
  };

  size_t counter = 0;
  const auto word_callback = [&counter](const Word& word) -> bool {
	//std::wcout << L'\t' << ++counter << L' ' << word << L" inserted...\n";
	return true;
  };

  Seeder::seedFromFiles<Char, Word>(size, filenames, autocomplete, file_callback, word_callback);

  return time(NULL) - start;
}

void run_input_parser(Autocomplete<Char, Word>& autocomplete) {
  Word prefix;
  do {
	std::wcout << L'>';
	std::wcin >> prefix;

	if (prefix == L":q")
	  break;

	if (prefix == L":l") {
	  size_t new_suggestions_limit;
	  std::wcin >> new_suggestions_limit;
	  autocomplete.set_suggestions_limit(new_suggestions_limit);
	  continue;
	}

	if (prefix == L":p") {
	  autocomplete.printInDotFormat(std::wcout);
	  continue;
	}

	if (prefix == L":i") {
	  Word word;
	  std::wcin >> word;
	  autocomplete.insert(word);
	  continue;
	}

	if (prefix == L":h") {
	  std::wcout << ":q - exit\n"
				 << ":l n - changes suggestions limit to n\n"
				 << ":p - prints the autocomplete data structure as DOT format\n"
				 << ":i word - adds word to autocomplete\n"
				 << ":h - help\n";
	  continue;
	}

	bool suggested = autocomplete.suggest(prefix, [&prefix](const Word& word) {
	  std::wcout << '\t' << prefix << word << "\n";
	});

	if (!suggested)
	  std::wcout << "Nothing to suggest for " << prefix << std::endl;
  } while (true);
}

int main(int argc, char** argv) {
  if (argc <= 1) {
	std::cout << "No words data set supplied. Run with some command line arguments" << std::endl;
	return 0;
  }

  Autocomplete<Char, Word> autocomplete;

  std::cout << "Processing words...\n";
  time_t elapsed_time = seed_autocomplete(static_cast<unsigned int>(argc - 1), argv + 1, autocomplete);
  std::cout << "Stats: \n";
  std::cout << "\tSeeding finished in " << elapsed_time << 's' << std::endl;
  std::cout << "\tTotal words seeded " << autocomplete.wordCount() << std::endl;

  //TODO: delete this line
  std::cout << '\t' << autocomplete.stateCount() << " state" << (autocomplete.stateCount() == 1 ? "" : "s")
			<< std::endl;

  if (!autocomplete.empty())
	run_input_parser(autocomplete);

  autocomplete.printInDotFormat(std::wcout);

  return 0;
}