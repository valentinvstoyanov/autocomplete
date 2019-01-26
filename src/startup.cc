#include <iostream>
#include "autocomplete.h"

int main() {
	std::cout << "Hello, world!" << std::endl;

	Autocomplete autocomplete;
	autocomplete.insert("abd");
	autocomplete.insert("bad");
	autocomplete.insert("bae");

	return 0;
}
