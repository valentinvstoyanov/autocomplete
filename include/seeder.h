#ifndef SEEDER_H
#define SEEDER_H

#include <fstream>
#include <string>

#include "autocomplete.h"

class Seeder {
 public:

  template<typename T>
  using WordCallback = std::function<bool(const T&)>;

  using FileCallback = std::function<bool(const char*, bool)>;

  template<typename Char = char, typename Word = std::basic_string<Char>>
  static bool seed(const Word& word,
				   Autocomplete<Char, Word>& autocomplete,
				   const WordCallback<Word>& word_callback) {
	autocomplete.insert(word);
	return word_callback(word);
  }

  template<typename Char = char, typename Word = std::basic_string<Char>>
  static bool seedFromFile(const char* filename,
						   Autocomplete<Char, Word>& autocomplete,
						   const FileCallback& file_callback,
						   const WordCallback<Word>& word_callback) {
	std::basic_ifstream<Char> file(filename);
	if (!file.is_open()) {
	  file_callback(filename, false);
	  return false;
	}

	if (file_callback(filename, true)) {
	  Word line;
	  while (std::getline(file, line) && seed<Char, Word>(line, autocomplete, word_callback));
	}

	return true;
  }

  template<typename Char = char, typename Word = std::basic_string<Char>>
  static bool seedFromFiles(unsigned size, char** const filenames,
							Autocomplete<Char, Word>& autocomplete,
							const FileCallback& file_callback,
							const WordCallback<Word>& word_callback) {
	for (unsigned i = 0; i < size; ++i)
	  seedFromFile<Char, Word>(filenames[i], autocomplete, file_callback, word_callback);

	return true;
  }
};

#endif //SEEDER_H

