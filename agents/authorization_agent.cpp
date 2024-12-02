#include "authorization_agent.hpp"
#include "keynodes/authorization_keynodes.hpp"
#include <optional>
#include <string>

ScAddr AuthorizationAgent::GetActionClass() const {
  return AuthorizationKeynodes::action_auth;
}

std::optional<AuthData> AuthorizationAgent::getArgs(ScAction &action) {
  auto const &[name_node, password_node] = action.GetArguments<2>();
  AuthData data;

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

bool AuthorizationAgent::validate_user(AuthData &data) {
  ScTemplate templ;
  templ.Triple(AuthorizationKeynodes::medicine_users,
               ScType::EdgeAccessVarPosPerm,
               ScType::NodeVar >> "_user_node");
  templ.Triple(AuthorizationKeynodes::concept_user,
               ScType::EdgeAccessVarPosPerm, "_user_node");
  templ.Quintuple("_user_node", ScType::EdgeDCommonVar,
                  ScType::LinkVar >> "lnk_password",
                  ScType::EdgeAccessVarPosPerm,
                  AuthorizationKeynodes::nrel_password);
  templ.Quintuple("_user_node", ScType::EdgeDCommonVar,
                  ScType::LinkVar >> "lnk_username",
                  ScType::EdgeAccessVarPosPerm,
                  AuthorizationKeynodes::nrel_username);

  ScTemplateSearchResult result;
  if (!m_context.SearchByTemplate(templ, result)) {
    std::cout << "There are no structures according to this template\n";
    return false;
  }

  for (int i = 0; i < result.Size(); i++) {
    std::string username;
    m_context.GetLinkContent(result[i]["lnk_username"], username);
    if (username == data.name) {
      std::string password;
      m_context.GetLinkContent(result[i]["lnk_password"], password);
      if (password == data.password) {
        return true;
      }
      return false;
    }
  }
  return false;
}

ScResult AuthorizationAgent::DoProgram(ScAction &action) {
  auto auth_data_opt = getArgs(action);
  if (!auth_data_opt.has_value()) {
    return action.FinishWithError();
  }
  AuthData auth_data = auth_data_opt.value();
  std::string result;
  if (validate_user(auth_data)) {
    result = "Valid";
  } else {
    result = "Invalid";
  }

  auto link_res = m_context.GenerateLink();
  m_context.SetLinkContent(link_res, result);
  action.FormResult(link_res);

  return action.FinishSuccessfully();
}
