#include "navigation_agent.hpp"
#include "keynodes/navigation_keynodes.hpp"
#include "sc-memory/sc_agent.hpp"
#include "sc-memory/sc_iterator.hpp"
#include "sc-memory/sc_template.hpp"
#include <cstddef>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <vector>

const std::string SUBJECT_DOMAIN = "subject_domain_of_diseases";
const std::string NREL_MAIN_IDTF = "nrel_main_idtf";
const int MAX_DIST = 75;

std::size_t hashEdgeRel(const EdgeRel &edge) { return edge.def_node.Hash(); }

void insert_new_node(edgeMap &edge_map, const EdgeRel &edge_rel,
                     std::string &node_name, ScType node_type) {
  auto it = edge_map.find(edge_rel);

  if (it != edge_map.end()) {
    it->second.push_back(Node{node_name, node_type});
  } else {
    edge_map.insert({edge_rel, {Node{node_name, node_type}}});
  }
}

ScAddr NavigationAgent::GetActionClass() const {
  return NavigationKeynodes::action_navigate;
}

std::optional<std::pair<std::string, bool>>
NavigationAgent::getArgName(ScAction &action) {
  auto const &[find_arg, lang_node] = action.GetArguments<2>();
  std::string node_name = "";
  std::string lang = "";

  if (!m_context.IsElement(find_arg) || !m_context.IsElement(lang_node)) {
    SC_AGENT_LOG_ERROR("There is no such node in the system!");
    return {};
  }

  if (!m_context.GetLinkContent(find_arg, node_name) ||
      !m_context.GetLinkContent(lang_node, lang)) {
    SC_AGENT_LOG_ERROR("No content in the link");
    return {};
  }
  bool is_russian = true;
  if (lang == "eng") {
    is_russian = false;
  }
  std::cout << "lang: " << is_russian << std::endl;

  return std::make_optional(std::make_pair(node_name, is_russian));
}

ScAddr NavigationAgent::getLinkAdj(ScAddr link_addr) {
  ScAddr main_idtf = m_context.ResolveElementSystemIdentifier(NREL_MAIN_IDTF);
  ScIterator5Ptr it5 = m_context.CreateIterator5(
      ScType::NodeConstClass, ScType::EdgeDCommonConst, link_addr,
      ScType::EdgeAccessConstPosPerm, main_idtf);
  if (it5->Next()) {
    return it5->Get(0);
  }

  return ScAddr::Empty;
}

bool NavigationAgent::isLinkedDomain(ScAddr link_addr) {
  std::unordered_map<ScAddr::HashType, size_t> node_dist;
  std::stack<ScAddr> addr_stack;

  addr_stack.push(link_addr);
  ScAddr domain = m_context.ResolveElementSystemIdentifier(SUBJECT_DOMAIN);
  auto domain_hash = domain.Hash();
  node_dist.insert({link_addr.Hash(), 0});
  while (!addr_stack.empty()) {
    ScAddr node = addr_stack.top();
    addr_stack.pop();

    if (domain_hash == node.Hash()) {
      return true;
    }

    ScIterator3Ptr it3 =
        m_context.CreateIterator3(ScType::Node, ScType::Unknown, node);
    while (it3->Next()) {
      ScAddr adj_addr = it3->Get(0);
      if (node_dist.find(adj_addr.Hash()) == node_dist.end()) {
        size_t next_distance = node_dist[node.Hash()] + 1;
        if (next_distance < MAX_DIST) {
          node_dist.insert({adj_addr.Hash(), next_distance});
          addr_stack.push(adj_addr);
        }
      }
    }
  }
  return false;
}

bool NavigationAgent::findLinks(ScAction &action, std::string &node_name,
                                bool is_russian) {
  auto nodes_set = m_context.SearchLinksByContent(node_name);
  std::string result = "";
  auto link_res = m_context.GenerateLink();
  // SC_AGENT_LOG_INFO("Size: " + std::to_string(nodes_set.size()));
  if (nodes_set.size() == 0) {
    return false;
  }
  bool found_linked = false;
  for (auto node : nodes_set) {
    ScAddr adj_node = getLinkAdj(node);
    if (adj_node == ScAddr::Empty) {
      std::cout << "Found not valid, skip" << std::endl;
      continue;
    }
    // saved_addr.push_back(adj_node);
    if (!isLinkedDomain(adj_node)) {
      std::cout << "Found not in our domain, skip" << std::endl;
      continue;
    }
    found_linked = true;
    result = getNodeElements(adj_node, is_russian);
    break;
  }
  if (!found_linked) {
    return false;
  }
  m_context.SetLinkContent(link_res, result);
  action.FormResult(link_res);

  return true;
}

std::string NavigationAgent::getOut5(ScAddr node_addr, ScType edgeType,
                                     edgeSet &edge_set, edgeMap &edge_map,
                                     bool is_russian) {
  std::string result;
  ScIterator5Ptr it5 =
      m_context.CreateIterator5(node_addr, edgeType, ScType::Node,
                                ScType::EdgeAccessConstPosPerm, ScType::Node);

  while (it5->Next()) {
    auto edge = it5->Get(1);
    auto node = it5->Get(2);
    auto def_node = it5->Get(4);
    auto def_node_str = m_context.GetElementSystemIdentifier(def_node);
    if (def_node_str == "" || def_node_str == "system_element" ||
        def_node_str == "rrel_1" ||
        edge_set.find(edge.Hash()) != edge_set.end()) {
      continue;
    }
    auto node_str = getName(node, is_russian);
    edge_set.insert(edge.Hash());
    ScType def_node_type = m_context.GetElementType(def_node);
    ScType edge_type = m_context.GetElementType(edge);
    ScType node_type = m_context.GetElementType(node);
    insert_new_node(edge_map, EdgeRel(def_node, def_node_type, edge_type, true),
                    node_str, node_type);

    // SC_AGENT_LOG_INFO("FRONT: " + m_context.GetElementSystemIdentifier(node)
    // +
    //                   ": " + m_context.GetElementSystemIdentifier(def_node));
  }
  return result;
}

std::string NavigationAgent::getIn5(ScAddr node_addr, ScType edgeType,
                                    edgeSet &edge_set, edgeMap &edge_map,
                                    bool is_russian) {
  std::string result;
  ScIterator5Ptr it5 =
      m_context.CreateIterator5(ScType::Node, edgeType, node_addr,
                                ScType::EdgeAccessConstPosPerm, ScType::Node);
  while (it5->Next()) {
    auto edge = it5->Get(1);
    auto node = it5->Get(0);
    auto def_node = it5->Get(4);
    auto def_node_str = m_context.GetElementSystemIdentifier(def_node);

    if (def_node_str == "" || def_node_str == "system_element" ||
        def_node_str == "rrel_key_sc_element" || def_node_str == "rrel_1" ||
        edge_set.find(edge.Hash()) != edge_set.end()) {
      continue;
    }

    edge_set.insert(edge.Hash());
    ScType def_node_type = m_context.GetElementType(def_node);
    ScType edge_type = m_context.GetElementType(edge);
    ScType node_type = m_context.GetElementType(node);
    auto node_str = getName(node, is_russian);
    insert_new_node(edge_map,
                    EdgeRel(def_node, def_node_type, edge_type, false),
                    node_str, node_type);

    // SC_AGENT_LOG_INFO("BACK: " + m_context.GetElementSystemIdentifier(node) +
    //                   ": " + m_context.GetElementSystemIdentifier(def_node));
  }
  return result;
}

std::string NavigationAgent::getOut3(ScAddr node_addr, ScType edgeType,
                                     edgeSet &edge_set,
                                     edgesNoRel &edges_no_rel,
                                     bool is_russian) {
  std::string result;
  ScIterator3Ptr it3 =
      m_context.CreateIterator3(node_addr, edgeType, ScType::Node);
  while (it3->Next()) {
    auto edge = it3->Get(1);
    auto node = it3->Get(2);
    auto node_str = m_context.GetElementSystemIdentifier(node);
    if (node_str == "" || edge_set.find(edge.Hash()) != edge_set.end()) {
      continue;
    }
    edge_set.insert(edge.Hash());
    auto edge_type = m_context.GetElementType(edge);
    auto node_type = m_context.GetElementType(node);
    edges_no_rel.emplace_back(
        std::tuple{EdgeRel(ScAddr::Empty, ScType::Unknown, edge_type, true),
                   Node(getName(node, is_russian), node_type)});
    // SC_AGENT_LOG_INFO("FRONT: " +
    // m_context.GetElementSystemIdentifier(node));
  }
  return result;
}

std::string NavigationAgent::getIn3(ScAddr node_addr, ScType edgeType,
                                    edgeSet &edge_set, edgesNoRel &edges_no_rel,
                                    bool is_russian) {
  std::string result;
  ScIterator3Ptr it3 =
      m_context.CreateIterator3(ScType::Node, edgeType, node_addr);
  while (it3->Next()) {
    auto edge = it3->Get(1);
    auto node = it3->Get(0);
    auto node_str = m_context.GetElementSystemIdentifier(node);
    if (node_str == "" || edge_set.find(edge.Hash()) != edge_set.end()) {
      continue;
    }
    edge_set.insert(edge.Hash());
    auto edge_type = m_context.GetElementType(edge);
    auto node_type = m_context.GetElementType(node);
    edges_no_rel.emplace_back(
        std::tuple{EdgeRel(ScAddr::Empty, ScType::Unknown, edge_type, true),
                   Node(getName(node, is_russian), node_type)});

    // SC_AGENT_LOG_INFO("BACK: " + m_context.GetElementSystemIdentifier(node));
  }
  return result;
}

std::string NavigationAgent::getDescription(ScAddr node_addr, bool is_russian) {
  ScAddr key_el =
      m_context.ResolveElementSystemIdentifier("rrel_key_sc_element");
  ScAddr definition = m_context.ResolveElementSystemIdentifier("definition");
  ScAddr nrel_translation =
      m_context.ResolveElementSystemIdentifier("nrel_sc_text_translation");
  std::string lang = is_russian ? "lang_ru" : "lang_en";
  ScAddr lang_node = m_context.ResolveElementSystemIdentifier(lang);
  ScTemplate templ;
  templ.Quintuple(ScType::NodeVar >> "_def_node", ScType::EdgeAccessVarPosPerm,
                  node_addr, ScType::EdgeAccessVarPosPerm, key_el);
  templ.Triple(definition, ScType::EdgeAccessVarPosPerm, "_def_node");
  templ.Quintuple("_def_node", ScType::EdgeDCommonVar,
                  ScType::NodeVar >> "_translations",
                  ScType::EdgeAccessVarPosPerm, nrel_translation);
  ;
  templ.Triple("_translations", ScType::EdgeAccessVarPosPerm,
               ScType::LinkVar >> "_link");
  templ.Triple(lang_node, ScType::EdgeAccessVarPosPerm, "_link");

  ScTemplateSearchResult templRes;
  m_context.SearchByTemplate(templ, templRes);
  if (templRes.Size() == 0) {
    return "";
  }

  std::string link_content;
  m_context.GetLinkContent(templRes[0]["_link"], link_content);
  return link_content;
}

std::string NavigationAgent::getName(ScAddr node_addr, bool is_russian) {
  ScAddr main_idtf = m_context.ResolveElementSystemIdentifier(NREL_MAIN_IDTF);
  std::string lang = is_russian ? "lang_ru" : "lang_en";
  ScAddr russian = m_context.ResolveElementSystemIdentifier(lang);
  ScTemplate templ;
  templ.Quintuple(node_addr, ScType::EdgeDCommonVar, ScType::LinkVar >> "_link",
                  ScType::EdgeAccessVarPosPerm, main_idtf);
  templ.Triple(russian, ScType::EdgeAccessVarPosPerm, "_link");
  ScTemplateSearchResult templRes;
  m_context.SearchByTemplate(templ, templRes);

  if (templRes.Size() == 0) {
    return m_context.GetElementSystemIdentifier(node_addr);
  }

  auto link = templRes[0]["_link"];
  std::string link_content;
  m_context.GetLinkContent(link, link_content);
  if (link_content == "") {
    link_content = "...";
  }

  return link_content;
}
std::string NavigationAgent::getNodeElements(ScAddr node_addr,
                                             bool is_russian) {
  edgeSet edge_set;
  edgeMap edge_map(10, hashEdgeRel);
  edgesNoRel edges_no_rel;

  getOut5(node_addr, ScType::EdgeDCommonConst, edge_set, edge_map, is_russian);
  getOut5(node_addr, ScType::EdgeUCommonConst, edge_set, edge_map, is_russian);
  getOut5(node_addr, ScType::EdgeAccessConstPosPerm, edge_set, edge_map,
          is_russian);
  getIn5(node_addr, ScType::EdgeDCommonConst, edge_set, edge_map, is_russian);
  getIn5(node_addr, ScType::EdgeUCommonConst, edge_set, edge_map, is_russian);
  getIn5(node_addr, ScType::EdgeAccessConstPosPerm, edge_set, edge_map,
         is_russian);
  // std::cout << "---------3--------" << std::endl;
  getOut3(node_addr, ScType::EdgeDCommonConst, edge_set, edges_no_rel,
          is_russian);
  getOut3(node_addr, ScType::EdgeAccessConstPosPerm, edge_set, edges_no_rel,
          is_russian);
  getIn3(node_addr, ScType::EdgeDCommonConst, edge_set, edges_no_rel,
         is_russian);
  getIn3(node_addr, ScType::EdgeAccessConstPosPerm, edge_set, edges_no_rel,
         is_russian);
  std::string result = "";
  result += getDescription(node_addr, is_russian) + "\n";
  std::cout << getDescription(node_addr, is_russian) << std::endl;
  for (auto &elem : edge_map) {
    result += elem.first.EdgeTypeString() +
              getName(elem.first.def_node, is_russian) + ":\n";
    std::cout << elem.first.EdgeTypeString()
              << getName(elem.first.def_node, is_russian) << ":\n";
    bool dot = false;
    if (elem.second.size() > 1) {
      dot = true;
    }
    for (auto &node : elem.second) {
      if (dot) {
        result += "* " + node.ToString() + "\n";
        std::cout << "* " + node.ToString() << "\n";
      } else {
        result += node.ToString() + "\n";
        std::cout << node.ToString() << "\n";
      }
    }
  }

  for (auto &elem : edges_no_rel) {
    result += std::get<0>(elem).EdgeTypeString() + " " +
              std::get<1>(elem).ToString() + "\n";
    std::cout << std::get<0>(elem).EdgeTypeString() << " "
              << std::get<1>(elem).ToString() << std::endl;
  }

  return result;
}

void NavigationAgent::form_result(ScAction &action, const std::string message) {
    auto link_res = m_context.GenerateLink();
    m_context.SetLinkContent(link_res, message);
    action.FormResult(link_res);
}

ScResult NavigationAgent::DoProgram(ScAction &action) {
  auto node_name_opt = getArgName(action);
  if (!node_name_opt.has_value()) {
    form_result(action, "Incorrect Input");
    return action.FinishUnsuccessfully();
  }
  std::pair<std::string, bool> pair_name = node_name_opt.value();
  auto node_name = pair_name.first;
  auto is_russian = pair_name.second;
  std::cout << "|" << node_name << "|\n" << "|" << is_russian << "|\n";

  ScAddr found_node = m_context.GenerateNode(ScType::NodeConst);

  bool is_found =
      m_context.SearchElementBySystemIdentifier(node_name, found_node);

  SC_AGENT_LOG_INFO("Pre check");
  if (!is_found) {
    SC_AGENT_LOG_INFO("There is no such node in the system!");
    if (!findLinks(action, node_name, is_russian)) {

      SC_AGENT_LOG_ERROR("No matches were found");
      form_result(action, "Not Found");
      return action.FinishSuccessfully();
    }
    return action.FinishSuccessfully();
  }
  SC_AGENT_LOG_INFO("Post check");

  if (!isLinkedDomain(found_node)) {
    SC_AGENT_LOG_ERROR("No matches were found");
    form_result(action, "Not Found");
    return action.FinishSuccessfully();
  }
  std::string result = getNodeElements(found_node, is_russian);

  form_result(action, result);

  return action.FinishSuccessfully();
}
