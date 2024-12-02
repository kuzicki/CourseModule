#include "course_module.hpp"
#include "agents/navigation_agent.hpp"
#include "agents/authorization_agent.hpp"
#include "agents/registration_agent.hpp"
#include "agents/blood_test_agent.hpp"
#include "agents/recomendation_agent.hpp"


SC_MODULE_REGISTER(CourseModule)
	->Agent<NavigationAgent>()
	->Agent<AuthorizationAgent>()
	->Agent<RegistrationAgent>()
	->Agent<BloodTestAgent>()
	->Agent<RecommendationAgent>();
