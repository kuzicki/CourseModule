#pragma once

#include <sc-memory/sc_keynodes.hpp>

class NavigationKeynodes : public ScKeynodes {
public:
  static inline ScKeynode const action_navigate{"action_navigate",
                                                ScType::NodeConstClass};
};
