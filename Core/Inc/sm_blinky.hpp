#ifndef INC_SM_BLINKY_HPP_
#define INC_SM_BLINKY_HPP_

#include "blinky_actions.h"
namespace sml = boost::sml;

namespace {

struct key_up {};
struct key_down {};
struct key_left {};
struct key_right {};

struct blue_button{};
struct timeout{};

// nonsense just try guard:
const auto is_key_down_valid = [](const key_down&) { return true; };

const auto turn_on = [] { Blinky_ON(); std::cout << "action: turning on" << std::endl; };
const auto turn_off = [] { Blinky_OFF(); std::cout << "action: turning off" << std::endl;};
const auto turn_pwm = [] { Blinky_PWM(); std::cout << "action: pwm" << std::endl;};

struct state_machine_blinky {
  auto operator()() const {

    using namespace sml;
    return make_transition_table(

      *"led pwm"_s + event<key_up> / turn_on = "led on"_s
	  ,"led pwm"_s + event<key_down> / turn_off = "led off"_s
	  ,"led off"_s + event<key_up> / turn_on = "led on"_s
	  ,"led on"_s + event<key_down> / turn_off = "led off"_s
      ,"led off"_s + event<key_right>  / turn_pwm = "led pwm"_s
	  ,"led on"_s + event<key_right> / turn_pwm = "led pwm"_s

	  ,"led pwm"_s + sml::on_entry<_> / [] { std::cout << "pwm entry" << std::endl; }
	  ,"led pwm"_s + sml::on_exit<_>  / [] { std::cout << "pwm exit" << std::endl; }

      ,"timed wait"_s + event<timeout> / turn_off = X
    );
  }
};
}


#endif /* INC_SM_BLINKY_HPP_ */
