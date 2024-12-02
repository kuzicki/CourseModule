#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>

struct BloodData {
  float WBC;
  float RBC;
  float platelets;
  bool is_russian;
};

struct BloodDataRange {
public:
  bool isInit() {
    if (WBC_max == -1.f || WBC_min == -1.f || RBC_min == -1.f ||
        RBC_max == -1.f || platelets_max == -1.f || platelets_min == -1.f) {
      return false;
    }
    return true;
  }

public:
  float WBC_max = -1.f;
  float WBC_min = -1.f;
  float RBC_max = -1.f;
  float RBC_min = -1.f;
  float platelets_max = -1.f;
  float platelets_min = -1.f;
};

namespace std {
template <> struct hash<ScAddr> {
  std::size_t operator()(const ScAddr &addr) const { return addr.Hash(); }
};
} // namespace std

class BloodTestAgent : public ScActionInitiatedAgent {
public:
  ScAddr GetActionClass() const override;
  ScResult DoProgram(ScAction &action) override;

private:
  std::optional<BloodData> getArgs(ScAction &action);
  std::vector<ScAddr> checkBlood(BloodData &data);
  void addParameter(ScTemplate &templ, const ScKeynode &parameter,
                    const ScKeynode &unit_of_measurement, std::string name);
  float getLinkVal(ScAddr node);

  void updateData(const ScKeynode &parameter,
                  std::unordered_map<ScAddr, BloodDataRange> &data);

  std::string getName(ScAddr node_addr, bool is_russian);
};
