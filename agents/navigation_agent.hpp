#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum RelType { rrel, nrel };
enum EdgeType { const_pos, edge_d, edge_u };
enum NodeType { class_const, node_const, struct_const };

struct EdgeRel {
public:
  EdgeRel(ScAddr def_node, ScType node_type, ScType edge_type, bool is_out) {
    if (node_type == ScType::NodeConstNoRole) {
      type = RelType::nrel;
    } else if (node_type == ScType::NodeConstRole) {
      type = RelType::rrel;
    } else {
      std::cout << "Found unknown node!" << std::endl;
    }

    this->def_node = def_node;
    this->is_out = is_out;
    if (edge_type == ScType::EdgeDCommonConst) {
      this->edge_type = EdgeType::edge_d;
    } else if (edge_type == ScType::EdgeUCommonConst) {
      this->edge_type = EdgeType::edge_u;
    } else if (edge_type == ScType::EdgeAccessConstPosPerm) {
      this->edge_type = EdgeType::const_pos;
    } else {
      std::cout << "Found unknown edge!" << std::endl;
    }
  }
  bool operator==(const EdgeRel &other) const {
    return def_node == other.def_node && is_out == other.is_out &&
           edge_type == other.edge_type;
  }

  std::string EdgeTypeString() const {
    if (edge_type == EdgeType::edge_d ) {
      return is_out ? "=>" : "<=";
    } else if (edge_type == EdgeType::edge_u) {
      return "<=>";
    } else if (edge_type == EdgeType::const_pos) {
      return is_out ? "∊" : "∍";
    }
    return std::to_string(edge_type); 
  }

public:
  ScAddr def_node;
  RelType type;
  EdgeType edge_type;
  bool is_out;
};

struct Node {
public:
  Node(const std::string &name, ScType node_type) {
    if (node_type == ScType::NodeConst) {
      type = NodeType::node_const;
    } else if (node_type == ScType::NodeConstClass) {
      type = NodeType::class_const;
    } else if (node_type == ScType::NodeConstStruct) {
      type = NodeType::struct_const;
    } else {
      std::cout << "Found unknonwn node type" << std::endl;
    }
    this->name = name;
  }
  std::string ToString() const { 
    return name;
    // if (type == NodeType::node_const ) {
    //   return "* " + name;
    // } else if (type == NodeType::struct_const) {
    //   return "* " + name;
    // } else if (type == NodeType::class_const) {
    //   return name;
    // }
    // return std::to_string(type) + ". " + name; 
  }

public:
  std::string name;
  NodeType type;
};

std::size_t hashEdgeRel(const EdgeRel &edge);

using edgeSet = std::unordered_set<ScAddr::HashType>;
using edgeMap =
    std::unordered_map<EdgeRel, std::vector<Node>, decltype(&hashEdgeRel)>;

using edgesNoRel = std::vector<std::tuple<EdgeRel, Node>>;

class NavigationAgent : public ScActionInitiatedAgent {
public:
  ScAddr GetActionClass() const override;

  ScResult DoProgram(ScAction &action) override;

private:
  std::optional<std::pair<std::string, bool>> getArgName(ScAction &action);
  bool findLinks(ScAction &action, std::string &node_name, bool is_russian);
  std::string getNodeElements(ScAddr node_addr, bool is_russian);
  ScAddr getLinkAdj(ScAddr link_addr);
  bool isLinkedDomain(ScAddr link_addr);
  std::string getOut5(ScAddr node_addr, ScType edgeType, edgeSet &edge_set,
                      edgeMap &edge_rels, bool is_russian);
  std::string getIn5(ScAddr node_addr, ScType edgeType, edgeSet &edge_set,
                     edgeMap &edge_rels, bool is_russian);
  std::string getOut3(ScAddr node_addr, ScType edgeType, edgeSet &edge_set,
                      edgesNoRel &edges_no_rel, bool is_russian);
  std::string getIn3(ScAddr node_addr, ScType edgeType, edgeSet &edge_set,
                     edgesNoRel &edges_no_rel, bool is_russian);
  std::string getName(ScAddr node_addr, bool is_russian);
  std::string getDescription(ScAddr node_addr, bool is_russian);
};
