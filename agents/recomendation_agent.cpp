#include "recomendation_agent.hpp"
#include "keynodes/recomendation_agent_nodes.hpp"
#include <optional>
#include <string>

const std::string NREL_MAIN_IDTF = "nrel_main_idtf";

ScAddr RecommendationAgent::GetActionClass() const {
  return RecommendationAgentKeynodes::action_rec;
}

std::optional<std::string> RecommendationAgent::getArgs(ScAction &action) {
  auto const &[node] = action.GetArguments<1>();

  if (!m_context.IsElement(node)) {
    SC_AGENT_LOG_ERROR("There is no such nodes in the system!");
    return {};
  }

  std::string node_data;
  if (!m_context.GetLinkContent(node, node_data)) {
    SC_AGENT_LOG_ERROR("No content in the link");
    return {};
  }
  return node_data;
}

std::optional<ScAddr>
RecommendationAgent::find_node_by_link(std::string &node_name) {
  auto links_set = m_context.SearchLinksByContent(node_name);
  ScAddr main_idtf = m_context.ResolveElementSystemIdentifier(NREL_MAIN_IDTF);

  if (links_set.size() == 0) {
    return std::nullopt;
  }

  for (auto link : links_set) {
    ScTemplate templ;
    templ.Quintuple(ScType::NodeVarClass >> "_disease_node",
                    ScType::EdgeDCommonVar, link,
                    ScType::EdgeAccessVarPosPerm, main_idtf);
    templ.Triple(RecommendationAgentKeynodes::concept_disease,
                 ScType::EdgeAccessVarPosPerm, "_disease_node");
    ScTemplateSearchResult res;
    m_context.SearchByTemplate(templ, res);
    if (res.Size() != 0) {
      std::cout << "Found by link sys idtf: "
                << m_context.GetElementSystemIdentifier(res[0]["_disease_node"])
                << std::endl;
      return res[0]["_disease_node"];
    }
  }
  return std::nullopt;
}

ScResult RecommendationAgent::DoProgram(ScAction &action) {
  auto find_node_opt = getArgs(action);
  if (!find_node_opt.has_value()) {
    return action.FinishWithError();
  }
  std::string find_node = find_node_opt.value();

  ScAddr node;
  if (!m_context.SearchElementBySystemIdentifier(find_node, node)) {
    std::cout << "Calling the find_node_by_link func" << std::endl;
    std::optional<ScAddr> node_opt = find_node_by_link(find_node);
    if (!node_opt.has_value()) {
      return action.FinishWithError();
    } else {
      node = node_opt.value();
    }
  }

  ScIterator3Ptr it3 =
      m_context.CreateIterator3(RecommendationAgentKeynodes::concept_disease,
                                ScType::EdgeAccessConstPosPerm, node);
  if (!it3->IsValid()) {
    return action.FinishWithError();
  }
  std::cout << "Sys idtf: " << m_context.GetElementSystemIdentifier(node)
            << std::endl;

  ScIterator5Ptr it5 = m_context.CreateIterator5(
      node, ScType::EdgeDCommonConst, ScType::LinkConst,
      ScType::EdgeAccessConstPosPerm, RecommendationAgentKeynodes::nrel_rec);

  if (!it5->Next()) {
    auto link_res = m_context.GenerateLink();
    std::string result = "No recomendations";
    m_context.SetLinkContent(link_res, result);
    action.FormResult(link_res);
    return action.FinishWithError();
  }
  ScAddr rec_link = it5->Get(2);
  action.FormResult(rec_link);

  return action.FinishSuccessfully();
}