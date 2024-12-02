#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>

struct AuthData {
  std::string name;
  std::string password;
};

class AuthorizationAgent : public ScActionInitiatedAgent {
public:
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction &action) override;

private:
  std::string getName(ScAddr node_addr, bool is_russian);
  std::string getDescription(ScAddr node_addr, bool is_russian);
	std::optional<AuthData> getArgs(ScAction &action);
	bool validate_user(AuthData &data); 
};
