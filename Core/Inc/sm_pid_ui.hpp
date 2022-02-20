#pragma once

#include "sml.hpp"
#include <iostream>
#include "screen.h"

namespace sml = boost::sml;

const auto show_off_screen = [] { std::cout << "action: OFF" << std::endl; refresh_screen(SCREEN_OFF); };
const auto show_setpoint_screen = [] { std::cout << "action: set setpoint" << std::endl; refresh_screen(SCREEN_SETPOINT); };
const auto show_run_screen = [] { std::cout << "action: run" << std::endl; refresh_screen(SCREEN_RUN);  };
const auto show_P_screen = [] { std::cout << "action: set P" << std::endl; refresh_screen(SCREEN_SET_P); };
const auto show_I_screen = [] { std::cout << "action: set I" << std::endl; refresh_screen(SCREEN_SET_I); };
const auto show_D_screen = [] { std::cout << "action: set D" << std::endl; refresh_screen(SCREEN_SET_D); };

struct key_up {};
struct key_down {};
struct key_left {};
struct key_right {};

struct blue_button{};
struct timeout{};

struct state_machine_pid_ui {
  auto operator()() const {

    using namespace sml;
    return make_transition_table(

		*"OFF"_s + event<key_right> = "SETPOINT"_s
		,"SETPOINT"_s + event<key_right> = "RUN"_s
		,"RUN"_s + event<key_right> = "TUNE_P"_s
		,"TUNE_P"_s + event<key_right> = "TUNE_I"_s
		,"TUNE_I"_s + event<key_right> = "TUNE_D"_s

		,"SETPOINT"_s + event<key_left> = "OFF"_s
		,"RUN"_s + event<key_left> = "SETPOINT"_s
		,"TUNE_P"_s + event<key_left> = "RUN"_s
		,"TUNE_I"_s + event<key_left> = "TUNE_P"_s
		,"TUNE_D"_s + event<key_left> = "TUNE_I"_s

		,"OFF"_s + sml::on_entry<_> / show_off_screen
		,"SETPOINT"_s + sml::on_entry<_> / show_setpoint_screen
		,"RUN"_s + sml::on_entry<_> / show_run_screen

		,"TUNE_P"_s + sml::on_entry<_> / show_P_screen
		,"TUNE_I"_s + sml::on_entry<_> / show_I_screen
		,"TUNE_D"_s + sml::on_entry<_> / show_D_screen


//      ,"timed "_s + event<timeout> / turn_off = X
    );
  }
};

