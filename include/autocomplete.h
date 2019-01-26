#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <unordered_map>
#include <functional>
#include <optional>

#include <iostream>

class Autocomplete {

	using Char = char;
	using Word = std::basic_string<Char>;

	using State = int;
	using OptionalState = std::optional<State>;

	using TransitionMap = std::unordered_map<Char, State>;
	using StateMap = std::unordered_map<State, TransitionMap>;

	using StateIt = StateMap::iterator;
	using TransitionIt = TransitionMap::iterator;

	using ConstStateIt = StateMap::const_iterator;
	using ConstTransitionIt = TransitionMap::const_iterator;

	StateMap states;
	State start;
	

	OptionalState makeTransition(State state, Char ch) const {
		ConstStateIt state_it = states.find(state);

		if (state_it == states.end())
			return {};

		const TransitionMap& transitions = state_it->second;
		ConstTransitionIt transition_it = transitions.find(ch);
		
		if (transition_it == transitions.end())
			return {};
		else
			return transition_it->second;
	}

	OptionalState makeTransition(State state, const Word& word) const {
		for (Char ch: word) {
			if (OptionalState next = makeTransition(state, ch)) {
				state = *next;
			} else {
				return {};
			}	
		}	
		
		return state;
	}
	
	State commonPrefix(const Word& word, Word& result) const {
		State current = start;
		
		for (Char ch: word) {
			if (OptionalState next = makeTransition(current, ch)) {
				result.push_back(ch);
				current = *next;
			} else {
				return current;
			}	
		}

		return current;	
	}

	bool isFinal(State state) const {
		return state < 0;
	}

	bool isFinal(OptionalState state) const {
		return state ? isFinal(*state) : false;
	}

	bool recognizes(const Word& word) const {
		return isFinal(makeTransition(start, word));
	}

	bool isEpsilon(const Word& word) const {
		return word.empty();
	}

	bool areTheSame(State s1, State s2) const {
		return s1 == s2;
	}

  public:

	Autocomplete(): start(0) {}
	Autocomplete(const Autocomplete&) = default;
	Autocomplete& operator=(const Autocomplete&) = default;
	~Autocomplete() = default;

	void insert(const Word& word) {
		Word common_prefix;
		State common_state = commonPrefix(word, common_prefix);	
		Word remaining_suffix = word.substr(common_prefix.length());
	
		std::cout << common_prefix << '\n' << remaining_suffix << std::endl;

//		if (isEpsilon(remaining_suffix) ^ recognizes(common_prefix))
//			return;
		
//		State last (areTheSame(common_state, start)) 
	}	
};

#endif

