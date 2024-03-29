#pragma once
#include <string>

namespace server{
    namespace commands{
        constexpr auto gStartMeasure = "start_measure";
        constexpr auto gSetRange = "set_range";
        constexpr auto gStopMeasure = "stop_measure";
        constexpr auto gGetStatus = "get_status";
        constexpr auto gGetResult = "get_result";
    }// namespace commands

    constexpr auto gChannelNumber = 9;
    constexpr auto gSocketName = "/tmp/9Lq7BNBnBycd6nxy.socket";
    constexpr auto gEnd = "END";
}// namespace server