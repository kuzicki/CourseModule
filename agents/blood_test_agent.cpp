#include "blood_test_agent.hpp"
#include "keynodes/blood_test_nodes.hpp"
#include "sc-memory/sc_template.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <string_view>

const std::string NREL_MAIN_IDTF = "nrel_main_idtf";

ScAddr BloodTestAgent::GetActionClass() const {
  return BloodTestNodes::action_blood_test;
}

std::optional<BloodData> BloodTestAgent::getArgs(ScAction &action) {
  auto const &[WBC_node, RBC_node, platelets_node, lang_node] =
      action.GetArguments<4>();
  BloodData data;

  if (!m_context.IsElement(WBC_node) || !m_context.IsElement(RBC_node) ||
      !m_context.IsElement(platelets_node) || !m_context.IsElement(lang_node)) {
    SC_AGENT_LOG_ERROR("Not sufficient nodes");
    return {};
  }
  std::string lang;
  if (!m_context.GetLinkContent(WBC_node, data.platelets) ||
      !m_context.GetLinkContent(RBC_node, data.RBC) ||
      !m_context.GetLinkContent(platelets_node, data.WBC) ||
      !m_context.GetLinkContent(lang_node, lang)) {
    SC_AGENT_LOG_ERROR("Error on reading link content");
    return {};
  }
  if (lang == "eng") {
    data.is_russian = false;
  } else {
    data.is_russian = true;
  }

  return data;
}

float BloodTestAgent::getLinkVal(ScAddr node) {
  std::string name;
  if (!m_context.GetLinkContent(node, name)) {
    std::cout << "No link content" << std::endl;
    return -1.f;
  };
  std::string_view view = name;
  if (view.substr(0, 7) == "\"float:") {
    // std::cout << "erase" << std::endl;
    view.remove_prefix(7);
    view.remove_suffix(1);
  }

  // std::cout << "Content to convert: " << view << std::endl;
  return std::stof(std::string { view });
}

bool inRange(float min, float val, float max) {
  return min <= val && val <= max;
}

void BloodTestAgent::updateData(
    const ScKeynode &parameter,
    std::unordered_map<ScAddr, BloodDataRange> &data) {

  ScIterator5Ptr it5 = m_context.CreateIterator5(
      parameter, ScType::EdgeDCommonConst, ScType::LinkConst,
      ScType::EdgeAccessConstPosPerm, BloodTestNodes::nrel_min_value_for_sick);

  while (it5->Next()) {
    auto link = it5->Get(2);
    auto link_data = getLinkVal(link);
    ScIterator3Ptr struct_it3 = m_context.CreateIterator3(
        ScType::NodeConstStruct, ScType::EdgeAccessConstPosPerm, link);
    if (!struct_it3->Next()) {
      std::cout << "Didn't find" << std::endl;
    };
    auto node_struct = struct_it3->Get(0);
    auto node_disease = node_struct;
    auto it = data.find(node_disease);
    BloodDataRange new_data;
    if (it == data.end()) {
      if (parameter == BloodTestNodes::concept_WBC) {
        new_data.WBC_min = link_data;
      } else if (parameter == BloodTestNodes::concept_RBC) {
        new_data.RBC_min = link_data;
      } else if (parameter == BloodTestNodes::concept_platelets) {
        new_data.platelets_min = link_data;
      }
      data[node_disease] = new_data;
    } else {
      new_data = it->second;
      if (parameter == BloodTestNodes::concept_WBC) {
        new_data.WBC_min = link_data;
      } else if (parameter == BloodTestNodes::concept_RBC) {
        new_data.RBC_min = link_data;
      } else if (parameter == BloodTestNodes::concept_platelets) {
        new_data.platelets_min = link_data;
      }
      it->second = new_data;
    }
  }

  it5 = m_context.CreateIterator5(
      parameter, ScType::EdgeDCommonConst, ScType::LinkConst,
      ScType::EdgeAccessConstPosPerm, BloodTestNodes::nrel_max_value_for_sick);

  while (it5->Next()) {
    auto link = it5->Get(2);
    auto link_data = getLinkVal(link);
    ScIterator3Ptr struct_it3 = m_context.CreateIterator3(
        ScType::NodeConstStruct, ScType::EdgeAccessConstPosPerm, link);
    if (!struct_it3->Next()) {
      std::cout << "Didn't find" << std::endl;
    };
    auto node_struct = struct_it3->Get(0);
    auto node_disease = node_struct;
    auto it = data.find(node_disease);
    BloodDataRange new_data;
    if (it == data.end()) {
      if (parameter == BloodTestNodes::concept_WBC) {
        new_data.WBC_max = link_data;
      } else if (parameter == BloodTestNodes::concept_RBC) {
        new_data.RBC_max = link_data;
      } else if (parameter == BloodTestNodes::concept_platelets) {
        new_data.platelets_max = link_data;
      }
      data[node_disease] = new_data;
    } else {
      new_data = it->second;
      if (parameter == BloodTestNodes::concept_WBC) {
        new_data.WBC_max = link_data;
      } else if (parameter == BloodTestNodes::concept_RBC) {
        new_data.RBC_max = link_data;
      } else if (parameter == BloodTestNodes::concept_platelets) {
        new_data.platelets_max = link_data;
      }
      it->second = new_data;
    }
  }
}

std::vector<ScAddr> BloodTestAgent::checkBlood(BloodData &data) {
  std::vector<ScAddr> result;
  ScTemplate templ_RBC;
  std::unordered_map<ScAddr, BloodDataRange> disease_data;

  updateData(BloodTestNodes::concept_RBC, disease_data);
  updateData(BloodTestNodes::concept_WBC, disease_data);
  updateData(BloodTestNodes::concept_platelets, disease_data);

  auto it = disease_data.begin();
  while (it != disease_data.end()) {
    ScAddr disease_key = it->first;
    BloodDataRange range = it->second;
    if (!range.isInit()) {
      std::cout << "Something is missing" << std::endl;
    }
    // std::cout << "WBC: " << range.WBC_min << " " << data.WBC << " "
    //           << range.WBC_max << std::endl;
    // std::cout << "RBC: " << range.RBC_min << " " << data.RBC << " "
    //           << range.RBC_max << std::endl;
    // std::cout << "platelets: " << range.platelets_min << " " << data.platelets
    //           << " " << range.platelets_max << std::endl;

    if (inRange(range.WBC_min, data.WBC, range.WBC_max) &&
        inRange(range.RBC_min, data.RBC, range.RBC_max) &&
        inRange(range.platelets_min, data.platelets, range.platelets_max)) {

      result.push_back(disease_key);
    }
    it++;
  }
  return result;
}

std::string BloodTestAgent::getName(ScAddr node_addr, bool is_russian) {
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

ScResult BloodTestAgent::DoProgram(ScAction &action) {
  auto auth_data_opt = getArgs(action);
  if (!auth_data_opt.has_value()) {
    return action.FinishWithError();
  }
  BloodData data = auth_data_opt.value();
  std::vector<ScAddr> result = checkBlood(data);

  if (result.size() == 0) {
    ScAddr no_disease_lnk = m_context.GenerateLink();
    m_context.SetLinkContent(no_disease_lnk, "No disease");

    action.FormResult(no_disease_lnk);
    return action.FinishSuccessfully();
  }

  ScAddr disease_lnk = m_context.GenerateLink();
  std::string disease = getName(result[0], data.is_russian);
  m_context.SetLinkContent(disease_lnk, disease);
  action.FormResult(disease_lnk);

  for (int i = 1; i < result.size(); i++) {
    disease_lnk = m_context.GenerateLink();
    disease = getName(result[i], data.is_russian);
    m_context.SetLinkContent(disease_lnk, disease);
    action.UpdateResult(disease_lnk);
  }
  return action.FinishSuccessfully();
}
