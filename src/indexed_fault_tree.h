/// @file indexed_fault_tree.h
/// Classes and facilities to represent simplified fault trees wth event and
/// gate indices instead of ID names. This facility is designed to work with
/// FaultTreeAnalysis class.
#ifndef SCRAM_SRC_INDEXED_FAULT_TREE_H_
#define SCRAM_SRC_INDEXED_FAULT_TREE_H_

#include <map>
#include <set>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace scram {

/// @class Node
/// An abstract base class that represents a node in an indexed fault tree
/// graph. The index of the node is a unique identifier for the node.
/// The node holds a weak pointer to the parent that is managed by the parent.
class Node {
 public:
  /// Creates a graph node with its index assigned sequentially.
  Node();

  /// Creates a graph node with its index.
  ///
  /// @param[in] index An unique positive index of this node.
  ///
  /// @warning The index is not validated upon instantiation.
  explicit Node(int index);

  virtual ~Node() = 0;  ///< Abstract class.

  /// @returns The index of this node.
  inline int index() const { return index_; }

  /// @returns parents of this gate.
  inline const std::set<int>& parents() { return parents_; }

  /// Adds a parent of this gate.
  ///
  /// @param[in] index Positive index of the parent.
  inline void AddParent(int index) {
    assert(index > 0);
    parents_.insert(index);
  }

  /// Removes a parent of this gate.
  ///
  /// @param[in] index Positive index of the existing parent.
  inline void EraseParent(int index) {
    assert(index > 0);
    assert(parents_.count(index));
    parents_.erase(index);
  }

  /// Registers the visit time for this node upon tree traversal.
  /// This information can be used to detect dependencies.
  ///
  /// @param[in] time The current visit time of this node. It must be positive.
  ///
  /// @returns true if this node was previously visited.
  /// @returns false if this is visited and re-visited only once.
  bool Visit(int time) {
    assert(time > 0);
    if (!visits_[0]) {
      visits_[0] = time;
    } else if (!visits_[1]) {
      visits_[1] = time;
    } else {
      visits_[2] = time;
      return true;
    }
    return false;
  }

  /// @returns The time when this node was first encountered or entered.
  /// @returns 0 if no enter time is registered.
  inline int EnterTime() const { return visits_[0]; }

  /// @returns The exit time upon traversal of the tree.
  /// @returns 0 if no exit time is registered.
  inline int ExitTime() const { return visits_[1]; }

  /// @returns The last time this node was visited.
  /// @returns 0 if no last time is registered.
  inline int LastVisit() const { return visits_[2] ? visits_[2] : visits_[1]; }

  /// @returns false if this node was only visited once upon tree traversal.
  /// @returns true if this node was revisited at one more time.
  inline bool Revisited() const { return visits_[2] ? true : false; }

  /// @returns true if this node was visited at least once.
  /// @returns false if this node was never visited upon traversal.
  inline bool Visited() const { return visits_[0] ? true : false; }

  /// Clears all the visit information. Resets the visit times to 0s.
  inline void ClearVisits() { return std::fill(visits_, visits_ + 3, 0); }

 private:
  static int next_index_;  ///< Automatic indexation of the next new node.
  int index_;  ///< Index of this node.
  /// This is a traversal array containing first, second, and last visits.
  int visits_[3];
  std::set<int> parents_;  ///< Parents of this node.
};

/// @class Constant
/// Representation of a node that is a Boolean constant with True or False
/// state.
class Constant : public Node {
 public:
  /// Constructs a new constant indexed node.
  ///
  /// @param[in] state Binary state of the Boolean constant.
  explicit Constant(bool state);

  ///@returns The state of the constant.
  inline bool state() { return state_; }

 private:
  bool state_;  ///< The Boolean value for the constant state.
};

/// @class IBasicEvent
/// Indexed basic events in a indexed fault tree.
class IBasicEvent : public Node {
 public:
  IBasicEvent();
};

/// @enum GateType
/// Types of gates for representation, preprocessing, and analysis purposes.
enum GateType {
  kAndGate,  ///< Simple AND gate.
  kOrGate,  ///< Simple OR gate.
  kAtleastGate,  ///< Combination or Vote gate representation.
  kXorGate,  ///< Exclusive OR gate with two inputs.
  kNotGate,  ///< Boolean negation.
  kNandGate,  ///< NAND gate.
  kNorGate,  ///< NOR gate.
  kNullGate  ///< Special pass-through or NULL gate. This is not NULL set.
};

/// @enum State
/// State of a gate as a set of events with a logical operator.
/// This state helps detect null and unity sets that formed upon Boolean
/// operations.
enum State {
  kNormalState,  ///< The default case with any set that is not null or unity.
  kNullState,  ///< The set is null. This indicates no failure.
  kUnityState  ///< The set is unity. This set guarantees failure.
};

/// @class IGate
/// This indexed gate is for use in IndexedFaultTree.
/// Initially this gate can represent any type of gate or logic; however,
/// this gate can be only of OR and AND type at the end of all simplifications
/// and processing. This gate class helps to process the fault tree before
/// any complex analysis is done.
class IGate : public Node {
 public:
  /// Creates a gate with its index.
  ///
  /// @param[in] index An unique positive index of this gate.
  /// @param[in] type The type of this gate.
  ///
  /// @warning The index is not validated upon instantiation.
  IGate(int index, const GateType& type);

  /// @returns Type of this gate.
  inline const GateType& type() const { return type_; }

  /// Changes the gate type information. This function is expected to be used
  /// with only simple AND, OR, NOT, NULL gates.
  ///
  /// @param[in] t The type for this gate.
  inline void type(const GateType& t) {
    assert(t == kAndGate || t == kOrGate || t == kNotGate || t == kNullGate);
    type_ = t;
  }

  /// @returns Vote number.
  inline int vote_number() const { return vote_number_; }

  /// Sets the vote number for this gate. The function does not check if
  /// the gate type is ATLEAST; nor does it validate the number.
  ///
  /// @param[in] number The vote number of ATLEAST gate.
  inline void vote_number(int number) { vote_number_ = number; }

  /// @returns children of this gate.
  inline const std::set<int>& children() const { return children_; }

  /// Directly assigns children for this gate.
  ///
  /// @param[in] children A new set of children for this gate.
  inline void children(const std::set<int>& children) { children_ = children; }

  /// @returns The state of this gate.
  inline const State& state() const { return state_; }

  /// @returns true if this gate is set to be a module.
  /// @returns false if it is not yet set to be a module.
  inline bool IsModule() const { return module_; }

  /// This function is used to initiate this gate with children.
  /// It is assumed that children are passed in ascending order from another
  /// children set.
  ///
  /// @param[in] child A positive or negative index of a child.
  void InitiateWithChild(int child);

  /// Adds a child to this gate. Before adding the child, the existing
  /// children are checked for complements. If there is a complement,
  /// the gate changes its state and clears its children. This functionality
  /// only works with OR and AND gates.
  ///
  /// @param[in] child A positive or negative index of a child.
  ///
  /// @returns false if there is a complement of the child being added.
  /// @returns true if the addition of this child is successful.
  ///
  /// @warning This function does not indicate error for future additions in
  ///          case the state is nulled or becomes unity.
  bool AddChild(int child);

  /// Swaps an existing child to a new child. Mainly used for
  /// changing the logic of this gate or complementing the child.
  ///
  /// @param[in] existing_child An existing child to get swapped.
  /// @param[in] new_child A new child.
  ///
  /// @warning If there is an iterator for the children set, then
  ///          it may become unusable because the children set is manipulated.
  bool SwapChild(int existing_child, int new_child);

  /// Makes all children complement of themselves.
  /// This is a helper function to propagate a complement gate and apply
  /// De Morgan's Law.
  void InvertChildren();

  /// Replaces a child with the complement of it.
  /// This is a helper function to propagate a complement gate and apply
  /// De Morgan's Law.
  void InvertChild(int existing_child);

  /// Adds children of a child gate to this gate. This is a helper function for
  /// gate coalescing. The child gate of the same type is removed from the
  /// children list.
  ///
  /// @param[in] child_gate The gate which children to be added to this gate.
  ///
  /// @returns false if the final set is null or unity.
  /// @returns true if the addition is successful with a normal final state.
  bool JoinGate(IGate* child_gate);

  /// Clears all the children of this gate.
  inline void EraseAllChildren() { children_.clear(); }

  /// Removes a child from the children container. The passed child index
  /// must be in this gate's children container and initialized.
  ///
  /// @param[in] child The positive or negative index of the existing child.
  inline void EraseChild(int child) {
    assert(children_.count(child));
    children_.erase(child);
  }

  /// Sets the state of this gate to null and clears all its children.
  /// This function is expected to be used only once.
  inline void Nullify() {
    assert(state_ == kNormalState);
    state_ = kNullState;
    children_.clear();
  }

  /// Sets the state of this gate to unity and clears all its children.
  /// This function is expected to be used only once.
  inline void MakeUnity() {
    assert(state_ == kNormalState);
    state_ = kUnityState;
    children_.clear();
  }

  /// Turns this gate's module flag on. This should be one time operation.
  inline void TurnModule() {
    assert(!module_);
    module_ = true;
  }

 private:
  GateType type_;  ///< Type of this gate.
  State state_;  ///< Indication if this gate's state is normal, null, or unity.
  int vote_number_;  ///< Vote number for ATLEAST gate.
  std::set<int> children_;  ///< Children of the gate.
  bool module_;  ///< Indication of an independent module gate.
};

class Gate;
class Formula;

/// @class IndexedFaultTree
/// This class provides simpler representation of a fault tree
/// that takes into account the indices of events instead of ids and pointers.
class IndexedFaultTree {
 public:
  typedef boost::shared_ptr<IGate> IGatePtr;
  typedef boost::shared_ptr<Gate> GatePtr;

  /// Constructs a simplified fault tree with indices of nodes.
  ///
  /// @param[in] top_event_id The index of the top event of this tree.
  explicit IndexedFaultTree(int top_event_id);

  /// @returns The index of the top gate of this fault tree.
  inline int top_event_index() const { return top_event_index_; }

  /// Sets the index for the top gate.
  ///
  /// @param[in] index Positive index of the top gate.
  inline void top_event_index(int index) { top_event_index_ = index; }

  /// @returns The current top gate of the fault tree.
  inline const IGatePtr& top_event() const {
    assert(indexed_gates_.count(top_event_index_));
    return indexed_gates_.find(top_event_index_)->second;
  }

  /// Creates indexed gates with basic and house event indices as children.
  /// Nested gates are flattened and given new indices.
  /// It is assumed that indices are sequential starting from 1.
  ///
  /// @param[in] int_to_inter Container of gates and their indices including
  ///                         the top gate.
  /// @param[in] ccf_basic_to_gates CCF basic events that are converted to
  ///                               gates.
  /// @param[in] all_to_int Container of all events in this tree to index
  ///                       children of the gates.
  void InitiateIndexedFaultTree(
      const boost::unordered_map<int, GatePtr>& int_to_inter,
      const std::map<std::string, int>& ccf_basic_to_gates,
      const boost::unordered_map<std::string, int>& all_to_int);

  /// Determines the type of the index.
  ///
  /// @param[in] index Positive index.
  ///
  /// @returns true if the given index belongs to an indexed gate.
  ///
  /// @warning The actual existance of the indexed gate is not guaranteed.
  inline bool IsGateIndex(int index) const {
    assert(index > 0);
    return index >= kGateIndex_;
  }

  /// Adds a new indexed gate into the indexed fault tree's gate container.
  ///
  /// @param[in] gate A new indexed gate.
  inline void AddGate(const IGatePtr& gate) {
    assert(!indexed_gates_.count(gate->index()));
    indexed_gates_.insert(std::make_pair(gate->index(), gate));
  }

  /// Commonly used function to get indexed gates from indices.
  ///
  /// @param[in] index Positive index of a gate.
  ///
  /// @returns The pointer to the requested indexed gate.
  inline const IGatePtr& GetGate(int index) const {
    assert(index > 0);
    assert(index >= kGateIndex_);
    assert(indexed_gates_.count(index));
    return indexed_gates_.find(index)->second;
  }

  /// Creates a new indexed gate. The gate is added to the fault tree container.
  /// The index for the new gates are assigned sequentially and guaranteed to
  /// be unique.
  ///
  /// @param[in] type The type of the new gate.
  ///
  /// @returns Pointer to the newly created gate.
  inline IGatePtr CreateGate(const GateType& type) {
    IGatePtr gate(new IGate(++new_gate_index_, type));
    indexed_gates_.insert(std::make_pair(gate->index(), gate));
    return gate;
  }

 private:
  typedef boost::shared_ptr<Formula> FormulaPtr;

  /// Mapping to string gate types to enum gate types.
  static const std::map<std::string, GateType> kStringToType_;

  /// Processes a formula into new indexed gates.
  ///
  /// @param[in] index The index to be assigned to the new indexed gate.
  /// @param[in] formula The formula to be converted into a gate.
  /// @param[in] ccf_basic_to_gates CCF basic events that are converted to
  ///                               gates.
  /// @param[in] all_to_int Container of all events in this tree to index
  ///                       children of the gates.
  void ProcessFormula(int index,
                      const FormulaPtr& formula,
                      const std::map<std::string, int>& ccf_basic_to_gates,
                      const boost::unordered_map<std::string, int>& all_to_int);

  int top_event_index_;  ///< The index of the top gate of this tree.
  const int kGateIndex_;  ///< The starting gate index for gate identification.
  /// All gates of this tree including newly created ones.
  boost::unordered_map<int, IGatePtr> indexed_gates_;
  int new_gate_index_;  ///< Index for a new gate.
};

}  // namespace scram

#endif  // SCRAM_SRC_INDEXED_FAULT_TREE_H_
