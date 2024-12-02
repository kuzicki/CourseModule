#pragma once

#include <sc-memory/sc_keynodes.hpp>

class AuthorizationKeynodes : public ScKeynodes {
public:
  static inline ScKeynode const action_auth{"action_auth",
                                                ScType::NodeConstClass};
  static inline ScKeynode const medicine_users{"medicine_users_data",
                                               ScType::NodeConstStruct};
  static inline ScKeynode const concept_user{"concept_user",
                                             ScType::NodeConstClass};
  static inline ScKeynode const nrel_password{"nrel_password",
                                              ScType::NodeConstNoRole};
  static inline ScKeynode const nrel_username{"nrel_username",
                                              ScType::NodeConstNoRole};
};
