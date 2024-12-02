#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>

struct RegData {
  std::string name;
  std::string password;
};

class RegistrationAgent : public ScActionInitiatedAgent {
public:
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction &action) override;

private:
	std::optional<RegData> getArgs(ScAction &action);
	bool validate_user(RegData &data); 
  void initUser(RegData &data); 
  std::string createUser(RegData &data); 
};
