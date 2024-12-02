#pragma once

#include <sc-memory/sc_keynodes.hpp>

class RecommendationAgentKeynodes : public ScKeynodes {
public:
  static inline ScKeynode const action_rec{"action_rec",
                                           ScType::NodeConstClass};
  static inline ScKeynode const concept_disease{"concept_disease",
                                                ScType::NodeConstClass};
  static inline ScKeynode const nrel_rec{"nrel_recommendation",
                                          ScType::NodeConstNoRole};
};
