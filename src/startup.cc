#include <iostream>
#include <fstream>
#include <string>

#include "../include/autocomplete.h"
#include "../include/seeder.h"

void print_greeting() {
	std::cout << "Welcome to autocomplete console application!";
	std::cout << std::endl;
}

template <typename Char = char, typename Word = std::basic_string<Char>>
void seed_autocomplete(unsigned size, char** const filenames, Autocomplete<Char, Word>& ac) {
	const auto file_callback = [](const std::string& filename, bool success) -> bool {
		if (success)
			std::cout << "Reading from " << filename << "...\n";
		else
			std::cerr << "Failed to read from " << filename << "\n";

		return success;
	};

	size_t counter = 0;
	const auto word_callback = [&counter](const std::string& word) -> bool {
		std::cout << '\t' << ++counter << ' ' << word << " inserted...\n";
		return true;
	};

	Seeder::seedFromFiles(size, filenames, ac, file_callback, word_callback);

	//TODO
	std::cout << "words: " << counter << std::endl;
}

template <typename Char = char, typename Word = std::basic_string<Char>>
void run_input_parser(Autocomplete<Char, Word>& ac) {
	//TODO
}


int main(int argc, char** argv) {
	print_greeting();

	if (argc <= 1) {
		std::cout << "No words data set supplied. Run with some command line arguments" << std::endl;
		return 0;
	}

	Autocomplete autocomplete;
	seed_autocomplete(static_cast<unsigned int>(argc- 1), argv + 1, autocomplete);

	//TODO
	std::cout << "words in autocomplete: " << autocomplete.wordCount() << "\n";

	std::string prefix("zwitter");
	std::cout << "Suggestions for: " << prefix << "\n";
	std::vector<std::string> suggestions;
	autocomplete.suggest(prefix, suggestions);
	for (const std::string& str: suggestions)
		std::cout << '\t' << prefix << str << "\n";

	//autocomplete.print();

	run_input_parser(autocomplete);

	return 0;
}