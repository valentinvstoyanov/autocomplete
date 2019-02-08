#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <unordered_map>
#include <functional>
#include <optional>
#include <cassert>
#include <ostream>
#include <unordered_set>

template<typename Char = char, typename Word = std::basic_string<Char>>
  class Autocomplete {

	using ConstWordRef = const Word&;

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

	State start;

	StateMap states;
	std::vector<bool> info;

	size_t word_counter;
	size_t suggestions_limit;

	bool isFinal(State state) const {
	  assert(state < info.size() && "Cannot check if non-existing state is final");
	  return info[state];
	}

	bool isFinal(OptionalState state) const {
	  return state ? isFinal(*state) : false;
	}

	bool recognizes(ConstWordRef word) const {
	  return isFinal(delta(start, word));
	}

	bool isEpsilon(ConstWordRef word) const {
	  return word.empty();
	}

	bool compare(State s1, State s2) const {
	  return s1 == s2;
	}

	OptionalState delta(State state, Char ch) const {
	  auto state_it = states.find(state);

	  if (state_it == states.end())
		return {};

	  const TransitionMap& transitions = state_it->second;
	  auto transition_it = transitions.find(ch);

	  if (transition_it == transitions.end())
		return {};

	  return transition_it->second;
	}

	OptionalState delta(State state, ConstWordRef word) const {
	  for (Char ch: word) {
		if (OptionalState next = delta(state, ch))
		  state = *next;
		else
		  return {};
	  }

	  return state;
	}

	using WalkCallback = std::function<bool(State from, Char with, State to)>;

	State commonPrefixWalk(ConstWordRef word, const WalkCallback& callback) const {
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
	  info.push_back(final);
	  return state;
	}

	void makeFinal(State state) {
	  assert(state < info.size() && "Cannot make final non-existing state");
	  info[state] = true;
	}

	/*
	using EquivalenceClassIndex = size_t;
	using EquivalenceClassIndexContainer = std::vector<EquivalenceClassIndex>;
	using EquivalenceClass = std::unordered_set<State>;
	using EquivalenceClassIndexToClass = std::unordered_map<EquivalenceClassIndex, EquivalenceClass>;
	using StateToEquivalenceClassIndex = std::unordered_map<State, EquivalenceClassIndex>;

	struct Partition {
	  EquivalenceClassIndexToClass idx_to_class;
	  StateToEquivalenceClassIndex state_to_idx;
	  EquivalenceClassIndexContainer new_class_idxs;

	  void insert(State state, EquivalenceClassIndex idx) {
		idx_to_class[idx].insert(state);
		state_to_idx[state] = idx;
	  }

	  EquivalenceClass& classByIdx(EquivalenceClassIndex idx) {
		return idx_to_class[idx];
	  }

	  EquivalenceClassIndex classIdxByState(State state) const {
		auto it = state_to_idx.find(state);
		assert(it != state_to_idx.end() && "Cannot get class index from non-existing state");
		return it->second;
	  }

	  size_t size() const {
		return idx_to_class.size();
	  }

	  EquivalenceClassIndex nextIdx() const {
		return idx_to_class.size();
	  }
	};

	bool areEquivalent(State first, State second, Partition& partition) const {
	  const TransitionMap& first_transitions = states.find(first)->second;
	  const TransitionMap& second_transitions = states.find(second)->second;

	  if (first_transitions.size() != second_transitions.size())
		return false;

	  for (const auto& transition: first_transitions) {
		OptionalState next = delta(second, transition.first);
		if ((next && (partition.classIdxByState(*next) != partition.classIdxByState(transition.second))) || !next)
		  return false;
	  }

	  return true;
	}

	std::optional<EquivalenceClassIndex> findSuitableExistingClass(State current, Partition& cpartition, Partition& partition) const {
	  for (EquivalenceClassIndex class_index: partition.new_class_idxs)
		if (areEquivalent(*(partition.classByIdx(class_index).begin()), current, cpartition))
		  return class_index;

	  return {};
	}

	void partitionClass(EquivalenceClass& eq_class, Partition& partition, Partition& const_partition) const {
	  auto eq_class_it = eq_class.begin();
	  State representative = *eq_class_it;
	  ++eq_class_it;

	  while (eq_class_it != eq_class.end()) {
		State current = *eq_class_it;

		if (areEquivalent(representative, current, const_partition)) {
		  ++eq_class_it;
		  continue;
		}

		eq_class_it = eq_class.erase(eq_class_it);

		if (std::optional<EquivalenceClassIndex> class_idx = findSuitableExistingClass(current, const_partition, partition)) {
		  partition.insert(current, *class_idx);
		} else {
		  const EquivalenceClassIndex idx = partition.nextIdx();
		  partition.insert(current, idx);
		  partition.new_class_idxs.push_back(idx);
		}
	  }
	}

	void partitionClasses(Partition& partition) const {
	  Partition const_partition = partition;
	  const size_t initial_partition_size = partition.size();
	  for (size_t i = 0; i < initial_partition_size; ++i) {
		EquivalenceClass& eq_class = partition.idx_to_class[i];
		assert(!eq_class.empty() && "Equivalence class cannot be empty");
		if (eq_class.size() == 1)
		  continue;

		partitionClass(eq_class, partition, const_partition);
	  }
	}

	void buildAutocompleteFromPartition(Partition& partition) {
	  StateMap state_map;
	  std::vector<bool> state_info;

	  for (EquivalenceClassIndex i = 0; i < partition.size(); ++i) {
	    const EquivalenceClass& eq_class = partition.classByIdx(i);
		assert(!eq_class.empty() && "Equivalence class cannot be empty");
	    State representative = *(eq_class.begin());

	    const TransitionMap& transitions = states[representative];
	    for (const auto& transition: transitions) {
	      Char with = transition.first;
	      State to = transition.second;
	      state_map[i][with] = partition.classIdxByState(to);
	    }

	    state_info.push_back(isFinal(representative));
	  }

	  states = state_map;
	  info = state_info;
	  start = partition.classIdxByState(start);
	}

	void minimize() {
	  Partition partition;
	  for (size_t i = 0; i < info.size(); ++i) {
	    partition.insert(i, static_cast<EquivalenceClassIndex>(info[i]));
	    //std::cout << static_cast<EquivalenceClassIndex>(info[i]) << std::endl;
	  }

	  size_t previous_partition_size = partition.size();

	  while (true) {
*/
	   /* std::cout << "size: " << partition.size() << std::endl;
		for (auto& p: partition.idx_to_class) {
		  std::cout << "{ ";
		  for (auto state: p.second) {
			std::cout << state << ' ';
		  }
		  std::cout << "} ";
		}
		std::cout << "\n";*/

		/*partitionClasses(partition);

		if (partition.size() == previous_partition_size)
		  break;

		previous_partition_size = partition.size();
		partition.new_class_idxs.clear();
	  }

	  buildAutocompleteFromPartition(partition);
	}*/

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

   public:
	using WordCallback = std::function<void(ConstWordRef)>;
   private:

	void wordsFromState(State current,
						Word& word,
						size_t& counter,
						const WordCallback& word_callback) const {
	  if (isFinal(current)) {
		word_callback(word);
		++counter;
	  }

	  if (counter >= suggestions_limit)
		return;

	  auto state_it = states.find(current);
	  assert(state_it != states.end() && "Cannot get words from non-existing state");

	  const TransitionMap& transitions = state_it->second;
	  for (auto tr_it = transitions.begin(); counter < suggestions_limit && tr_it != transitions.end(); ++tr_it) {
		word += tr_it->first;
		const size_t old_word_len = word.length();
		wordsFromState(tr_it->second, word, counter, word_callback);
		word.erase(old_word_len - 1);
	  }
	}
   public:
	explicit Autocomplete(size_t suggestions_limit = 5)
		: start(0),
		  info(1, false),
		  states(StateMap()),
		  word_counter(0),
		  suggestions_limit(suggestions_limit) {
	  states[start];
	}

	Autocomplete(const Autocomplete&) = default;
	Autocomplete& operator=(const Autocomplete&) = default;
	~Autocomplete() = default;

	void insert(ConstWordRef word) {
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

	bool suggest(ConstWordRef prefix, const WordCallback& word_callback) const {
	  Word common_prefix;
	  State last = commonPrefixWalk(prefix, [&common_prefix](State from, Char with, State to) -> bool {
		common_prefix += with;
		return true;
	  });

	  if (prefix.length() != common_prefix.length() && !delta(last, prefix[common_prefix.length() - 1]))
		return false;

	  Word buff;
	  size_t counter = 0;
	  wordsFromState(last, buff, counter, word_callback);
	  return true;
	}

	//TODO: delete this horrible function
	void printInDotFormat(std::basic_ostream<Char>& out) const {
	  out << "digraph {\n";

	  for (auto state_it = states.begin(); state_it != states.end(); ++state_it) {
		const TransitionMap& tr_map = state_it->second;

		for (auto tr_it = tr_map.begin(); tr_it != tr_map.end(); ++tr_it)
		  out << '\t' << state_it->first << " -> " << tr_it->second << " [label=\"" << tr_it->first << "\"];\n";

		if (isFinal(state_it->first))
		  out << '\t' << state_it->first << "[color=chartreuse,style=filled, fillcolor=\"#e7f9d1\"];\n";
	  }

	  out << '\t' << start << "[color=blue1,style=filled, fillcolor=\"#cde3f7\"];\n}\n";
	}

	size_t wordCount() const {
	  return word_counter;
	}

	bool empty() const {
	  return word_counter == 0;
	}

	//TODO: delete this accessor
	size_t stateCount() const {
	  return states.size();
	}

	size_t suggestionsLimit() const {
	  return suggestions_limit;
	}

	void set_suggestions_limit(size_t value) {
	  suggestions_limit = value;
	}
  };

#endif

