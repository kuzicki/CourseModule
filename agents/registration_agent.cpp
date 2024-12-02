#include "registration_agent.hpp"
#include "keynodes/registration_keynodes.hpp"
#include "sc-memory/sc_template.hpp"
#include <optional>
#include <string>

ScAddr RegistrationAgent::GetActionClass() const {
  return RegistrationKeynodes::action_reg;
}

std::optional<RegData> RegistrationAgent::getArgs(ScAction &action) {
  auto const &[name_node, password_node] = action.GetArguments<2>();
  RegData data;

  if (!m_context.IsElement(name_node) || !m_context.IsElement(password_node)) {
    SC_AGENT_LOG_ERROR("There is no such nodes in the system!");
    return {};
  }

  if (!m_context.GetLinkContent(name_node, data.name) ||
      !m_context.GetLinkContent(password_node, data.password)) {
    SC_AGENT_LOG_ERROR("No content in the link");
    return {};
  }
  return data;
}

void RegistrationAgent::initUser(RegData &data) {
  auto link_username = m_context.GenerateLink();
  auto link_password = m_context.GenerateLink();
  m_context.SetLinkContent(link_username, data.name);
  m_context.SetLinkContent(link_password, data.password);
  ScTemplate templ;
  templ.Triple(RegistrationKeynodes::medicine_users,
               ScType::EdgeAccessVarPosPerm, ScType::NodeVar >> "_user_node");
  templ.Triple(RegistrationKeynodes::concept_user, ScType::EdgeAccessVarPosPerm,
               "_user_node");
  templ.Quintuple("_user_node", ScType::EdgeDCommonVar, link_username,
                  ScType::EdgeAccessVarPosPerm,
                  RegistrationKeynodes::nrel_username >> "_username");
  templ.Quintuple("_user_node", ScType::EdgeDCommonVar, link_password,
                  ScType::EdgeAccessVarPosPerm,
                  RegistrationKeynodes::nrel_password >> "_password");

  ScTemplateResultItem result;
  m_context.GenerateByTemplate(templ, result);
}

std::string RegistrationAgent::createUser(RegData &data) {

  ScTemplate templ;
  templ.Triple(RegistrationKeynodes::medicine_users,
               ScType::EdgeAccessVarPosPerm, ScType::NodeVar >> "_user_node");

  templ.Triple(RegistrationKeynodes::concept_user, ScType::EdgeAccessVarPosPerm,
               "_user_node");

  templ.Quintuple("_user_node", ScType::EdgeDCommonVar,
                  ScType::LinkVar >> "lnk_username",
                  ScType::EdgeAccessVarPosPerm,
                  RegistrationKeynodes::nrel_username >> "_username");

  templ.Quintuple("_user_node", ScType::EdgeDCommonVar, ScType::LinkVar,
                  ScType::EdgeAccessVarPosPerm,
                  RegistrationKeynodes::nrel_password >> "_password");

  ScTemplateSearchResult result;
  if (!m_context.SearchByTemplate(templ, result)) {
    initUser(data);
    return "User created";
  }

  bool user_exists = false;
  for (int i = 0; i < result.Size(); i++) {
    std::string username;
    m_context.GetLinkContent(result[i]["lnk_username"], username);
    if (username == data.name) {
      user_exists = true;
    }
  }

  if (user_exists) {
    return "User exists";
  }

  initUser(data);
  return "User created";
}

ScResult RegistrationAgent::DoProgram(ScAction &action) {
  auto auth_data_opt = getArgs(action);
  if (!auth_data_opt.has_value()) {
    return action.FinishWithError();
  }
  RegData reg_data = auth_data_opt.value();
  std::string result = createUser(reg_data);

  auto link_res = m_context.GenerateLink();
  m_context.SetLinkContent(link_res, result);
  action.FormResult(link_res);

  return action.FinishSuccessfully();
}
