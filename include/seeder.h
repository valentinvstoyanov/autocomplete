#ifndef SEEDER_H
#define SEEDER_H

#include <fstream>
#include <string>

#include "autocomplete.h"

class Seeder {
  public:
	using WordCallback = std::function<bool(const std::string&)>;
	using FileCallback = std::function<bool(const std::string&, bool)>;

	template <typename Char = char, typename Word = std::basic_string<Char>>
	static bool seed(const std::string& word,
					 Autocomplete<Char, Word>& autocomplete,
					 const WordCallback& word_callback) {
		autocomplete.insert(word);
		return word_callback(word);
	}

	template <typename Char = char, typename Word = std::basic_string<Char>>
	static bool seedFromFile(const std::string& filename,
							 Autocomplete<Char, Word>& autocomplete,
							 const FileCallback& file_callback,
							 const WordCallback& word_callback) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			file_callback(filename, false);
			return false;
		}

		file_callback(filename, true);
		std::string line;
		while (std::getline(file, line) && seed(line, autocomplete, word_callback));

		return true;
	}

	template <typename Char = char, typename Word = std::basic_string<Char>>
	static bool seedFromFiles(unsigned size, char** const filenames,
							  Autocomplete<Char, Word>& autocomplete,
							  const FileCallback& file_callback,
							  const WordCallback& word_callback) {
		for (unsigned i = 0; i<size; ++i)
			seedFromFile(filenames[i], autocomplete, file_callback, word_callback);

		return true;
	}
};

#endif //SEEDER_H

