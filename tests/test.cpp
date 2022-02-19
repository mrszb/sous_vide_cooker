#include "../Core/Inc/sml.hpp"
#include "../Core/Inc/sm_blinky.hpp"

#include "catch_amalgamated.hpp"

using namespace boost::sml;

TEST_CASE( "init sm", "[single-file]" ) {
    sm<state_machine_blinky> sm;
	REQUIRE(sm.is("led pwm"_s));
}