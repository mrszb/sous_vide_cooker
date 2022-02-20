#pragma once

#include "sml.hpp"
#include <iostream>
#include "screen.h"

extern void change_param(const char* name, int dir);

namespace sml = boost::sml;

const auto show_off_screen = [] { std::cout << "action: OFF" << std::endl; refresh_screen(SCREEN_OFF); };
const auto show_setpoint_screen = [] { std::cout << "action: set setpoint" << std::endl; refresh_screen(SCREEN_SETPOINT); };
const auto show_run_screen = [] { std::cout << "action: run" << std::endl; refresh_screen(SCREEN_RUN);  };
const auto show_P_screen = [] { std::cout << "action: set P" << std::endl; refresh_screen(SCREEN_SET_P); };
const auto show_I_screen = [] { std::cout << "action: set I" << std::endl; refresh_screen(SCREEN_SET_I); };
const auto show_D_screen = [] { std::cout << "action: set D" << std::endl; refresh_screen(SCREEN_SET_D); };

const auto run_up = [] {std::cout << "action: run up" << std::endl; change_param("RUN", 1); refresh_screen(SCREEN_RUN); };
const auto run_down = [] {std::cout << "action: run down" << std::endl; change_param("RUN", -1); refresh_screen(SCREEN_RUN); };

const auto setpoint_up = [] {std::cout << "action: setpoint up" << std::endl; change_param("SP", 1); refresh_screen(SCREEN_SETPOINT); };
const auto setpoint_down = [] {std::cout << "action: setpoint down" << std::endl; change_param("SP", -1); refresh_screen(SCREEN_SETPOINT); };

const auto kp_up = [] {std::cout << "action: Kp up" << std::endl; change_param("P", 1); refresh_screen(SCREEN_SET_P); };
const auto kp_down = [] {std::cout << "action: Kp down" << std::endl; change_param("P", -1); refresh_screen(SCREEN_SET_P); };
const auto ki_up = [] {std::cout << "action: Ki up" << std::endl; change_param("I", 1); refresh_screen(SCREEN_SET_I); };
const auto ki_down = [] {std::cout << "action: Ki down" << std::endl; change_param("I", -1); refresh_screen(SCREEN_SET_I); };
const auto kd_up = [] {std::cout << "action: Kd up" << std::endl; change_param("D", 1); refresh_screen(SCREEN_SET_D); };
const auto kd_down = [] {std::cout << "action: Kd down" << std::endl; change_param("D", -1); refresh_screen(SCREEN_SET_D); };

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

		,"SETPOINT"_s + event<key_up> / setpoint_up
		,"SETPOINT"_s + event<key_down> / setpoint_down

		,"RUN"_s + event<key_up> / run_up
		,"RUN"_s + event<key_down> / run_down

		,"TUNE_P"_s + event<key_up> / kp_up
		,"TUNE_P"_s + event<key_down> / kp_down

		,"TUNE_I"_s + event<key_up> / ki_up
		,"TUNE_I"_s + event<key_down> / ki_down

		,"TUNE_D"_s + event<key_up> / kd_up
		,"TUNE_D"_s + event<key_down> / kd_down


//      ,"timed "_s + event<timeout> / turn_off = X
    );
  }
};

