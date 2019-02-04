#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <cassert>
#include <map>

#include <iostream>

template<typename Char = char, typename Word = std::basic_string<Char>>
  class Autocomplete {

	using ConstWordRef = const Word &;

	using State = unsigned long;
	using OptionalState = std::optional<State>;

	using Transition = std::pair<State, Char>;
	using OptionalTransition = std::optional<Transition>;

	using TransitionMap = std::unordered_map<Char, State>;
	using StateMap = std::unordered_map<State, TransitionMap>;

	using StateIt = typename StateMap::iterator;
	using TransitionIt = typename TransitionMap::iterator;

	using ConstStateIt = typename StateMap::const_iterator;
	using ConstTransitionIt = typename TransitionMap::const_iterator;

	StateMap states;
	State start;
	std::vector<char> states_info;
	size_t word_counter;
	size_t suggestions_limit;

	static constexpr char kFinalCh = 'f';
	static constexpr char kConfluenceCh = 'c';
	static constexpr char kBothCh = 'b';
	static constexpr char kEmptyCh = '\0';

	bool isFinal(State state) const {
	  assert(state < states_info.size() && "Cannot check if non-existing state is final");
	  return states_info[state]==kFinalCh || states_info[state]==kBothCh;
	}

	bool isFinal(OptionalState state) const {
	  return state ? isFinal(*state) : false;
	}

	bool isConfluence(State state) const {
	  assert(state < states_info.size() && "Cannot check if non-existing state is confluence");
	  return states_info[state]==kConfluenceCh || states_info[state]==kBothCh;
	}

	bool isConfluence(OptionalState state) const {
	  return state ? isConfluence(*state) : false;
	}

	bool recognizes(ConstWordRef word) const {
	  return isFinal(delta(start, word));
	}

	bool isEpsilon(ConstWordRef word) const {
	  return word.empty();
	}

	bool compare(State s1, State s2) const {
	  return s1==s2;
	}

	OptionalState delta(State state, Char ch) const {
	  auto state_it = states.find(state);

	  if (state_it==states.end())
		return {};

	  const TransitionMap &transitions = state_it->second;
	  auto transition_it = transitions.find(ch);

	  if (transition_it==transitions.end())
		return {};

	  return transition_it->second;
	}

	OptionalState delta(State state, ConstWordRef word) const {
	  for (Char ch: word) {
		if (OptionalState next = delta(state, ch)) {
		  state = *next;
		} else {
		  return {};
		}
	  }

	  return state;
	}

	State commonPrefixWalk(ConstWordRef word,
						   const std::function<bool(State from, Char with, State to)> &callback) const {
	  State current = start;

	  for (Char ch : word) {
		ch = tolower(ch);
		if (OptionalState next = delta(current, ch)) {
		  if (!callback(current, ch, *next))
			return *next;
		  current = *next;
		} else {
		  return current;
		}
	  }

	  return current;
	}

	State newState(bool final = false) {
	  State state(states.size());
	  states[state];
	  states_info.push_back(final ? kFinalCh : kEmptyCh);
	  return state;
	}

	void makeFinal(State state) {
	  assert(state < states_info.size() && "Cannot make final non-existing state");
	  states_info[state] = states_info[state]==kConfluenceCh ? kBothCh : kFinalCh;
	}

	State clone(State state) {
	  auto state_it = states.find(state);
	  assert(state_it!=states.end() && "Cannot clone non-existing state");

	  State cloned = newState(isFinal(state));
	  const TransitionMap &transitions = state_it->second;
	  for (const auto &transition : transitions)
		states[cloned][transition.first] = transition.second;

	  return cloned;
	}

	void addSuffix(State from, ConstWordRef suffix) {
	  if (isEpsilon(suffix)) {
		makeFinal(from);
		return;
	  }

	  for (size_t i = 0; i < suffix.length() - 1; ++i) {
		State next = newState();
		states[from][suffix[i]] = next;
		from = next;
	  }
	  states[from][suffix.back()] = newState(true);
	}

	void wordsFromState(State current, Word &word, std::vector<Word> &result) const {
	  if (isFinal(current)) {
		result.push_back(word);

		if (result.size()==suggestions_limit)
		  return;
	  }

	  auto state_it = states.find(current);
	  assert(state_it!=states.end() && "Cannot get words from non-existing state");

	  const TransitionMap &transitions = state_it->second;
	  for (auto tr_it = transitions.begin(); tr_it!=transitions.end(); ++tr_it) {
		word += tr_it->first;
		int prev_word_len = word.length();
		wordsFromState(tr_it->second, word, result);
		word.erase(prev_word_len - 1);
	  }
	}
   public:

	explicit Autocomplete(size_t suggestions_limit = 5)
		: start(0),
		  states_info(1, kEmptyCh),
		  states(StateMap()), word_counter(0),
		  suggestions_limit(suggestions_limit) {
	  states[start];
	}

	Autocomplete(const Autocomplete &) = default;
	Autocomplete &operator=(const Autocomplete &) = default;
	~Autocomplete() = default;

	void insert(ConstWordRef word) {
	  /*
	  OptionalState confluence;
   Word common_prefix;

   State last = commonPrefixWalk(word,
		   [this, &confluence, &common_prefix](State from, Char with, State to) -> bool {
			   common_prefix.push_back(with);

			   if (!confluence && isConfluence(to))
				   confluence = to;

			   return true;
		   });

   Word remaining_suffix = word.substr(common_prefix.length());

   if (isEpsilon(remaining_suffix) && isFinal(last))
	   return;

   if (confluence)
	   last = clone(last);

   addSuffix(last, remaining_suffix);*/


	  Word prefix;
	  State last = commonPrefixWalk(word, [&prefix](State from, Char with, State to) -> bool {
		prefix += with;
		return true;
	  });

	  Word suffix = word.substr(prefix.length());

	  if (isEpsilon(suffix) && isFinal(last))
		return;

	  addSuffix(last, suffix);
	  ++word_counter;
	}

	void suggest(ConstWordRef prefix, std::vector<Word> &result) const {
	  result.clear();

	  Word common_prefix;
	  State last = commonPrefixWalk(prefix, [&common_prefix](State from, Char with, State to) -> bool {
		common_prefix += with;
		return true;
	  });

	  Word buff;
	  wordsFromState(last, buff, result);
	}

	//TODO: delete this horrible function
	void print() const {
	  std::cout << "digraph {\n";

	  for (auto nit = states.begin(); nit!=states.end(); ++nit) {
		TransitionMap &emap = nit->second;

		for (auto eit = emap.begin(); eit!=emap.end(); ++eit)
		  std::cout << '\t' << nit->first << " -> " << eit->second << " [label=\"" << eit->first << "\"];\n";

		if (isFinal(nit->first))
		  std::cout << '\t' << nit->first << "[color=red,style=filled, fillcolor=\"#ffefef\"];\n";
	  }

	  std::cout << "}\n";
	}

	size_t wordCount() const {
	  return word_counter;
	}

	size_t suggestionsLimit() const {
	  return suggestions_limit;
	}

	void set_suggestions_limit(size_t value) {
	  suggestions_limit = value;
	}
  };

#endif

