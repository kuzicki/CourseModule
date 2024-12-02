#pragma once

#include <sc-memory/sc_keynodes.hpp>

class BloodTestNodes : public ScKeynodes {
public:
  static inline ScKeynode const action_blood_test{"action_blood_test",
                                                  ScType::NodeConstClass};
  static inline ScKeynode const concept_WBC{"concept_WBC",
                                                   ScType::NodeConstClass};
  static inline ScKeynode const number{"number", ScType::NodeConstClass};
  static inline ScKeynode const millionuL{"millionuL", ScType::NodeConstClass};
  static inline ScKeynode const uL{"uL", ScType::NodeConstClass};
  static inline ScKeynode const concept_RBC{"concept_RBC",
                                                ScType::NodeConstClass};
  static inline ScKeynode const concept_platelets{"concept_platelets",
                                                ScType::NodeConstClass};
  static inline ScKeynode const nrel_unit_of_measurement{
      "nrel_unit_of_measurement", ScType::NodeConstNoRole};
  static inline ScKeynode const nrel_min_value_for_sick{
      "nrel_min_value_for_sick", ScType::NodeConstNoRole};
  static inline ScKeynode const nrel_max_value_for_sick{
      "nrel_max_value_for_sick", ScType::NodeConstNoRole};
};
