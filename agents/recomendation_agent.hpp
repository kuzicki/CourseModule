#pragma once

#include <optional>
#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>


class RecommendationAgent : public ScActionInitiatedAgent {
public:
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction &action) override;

private:
  std::optional<std::string> getArgs(ScAction &action);
  std::optional<ScAddr> find_node_by_link(std::string &node_name);
  void form_result(ScAction &action, const std::string message);
};
