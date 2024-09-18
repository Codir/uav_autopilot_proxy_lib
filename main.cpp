/*#include "mavsdk_interface.h"

int main() {
    LinkAutopilotInterface mavsdk_interface = LinkAutopilotInterface{};
    // Подключаемся к SITL через UDP
    if (!mavsdk_interface.connect("udp://:14540")) {
        return 1;
    }

    // Устанавливаем режим GUIDED_NO_GPS
    mavsdk_interface.setModeGuidedNoGPS();

    // Армируем дрон
    mavsdk_interface.armThrottle();

    // Взлетаем на высоту 5 метров
    mavsdk_interface.takeoff(5.0f);

    return 0;
}*/


//
// Simple example to demonstrate how takeoff and land using MAVSDK.
//

#include <chrono>
#include <cstdint>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <iostream>
#include <future>
#include <memory>
#include <thread>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

int main()
{
    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection("udp://:14550");

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << '\n';
        return 1;
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system) {
        std::cerr << "Timed out waiting for system\n";
        return 1;
    }

    // Instantiate plugins.
    auto telemetry = Telemetry{system.value()};
    auto action = Action{system.value()};
    auto mavlink_passthrough = new MavlinkPassthrough{system.value()};

    // We want to listen to the altitude of the drone at 1 Hz.
    const auto set_rate_result = telemetry.set_rate_position(1.0);
    if (set_rate_result != Telemetry::Result::Success) {
        std::cerr << "Setting rate failed: " << set_rate_result << '\n';
        return 1;
    }

    // Set up callback to monitor altitude while the vehicle is in flight
    telemetry.subscribe_position([](Telemetry::Position position) {
        std::cout << "Altitude: " << position.relative_altitude_m << " m\n";
    });

    // Check until vehicle is ready to arm
    while (telemetry.health_all_ok() != true) {
        std::cout << "Vehicle is getting ready to arm\n";
        sleep_for(seconds(1));
    }

//
    //mavlink_message_t message;
    uint16_t command = MAV_CMD_DO_SET_MODE;  // Команда для установки режима
    float custom_mode = 4; //Mode::Number::GUIDED;  // Указываем нужный режим (например, 4 для GUIDED)

    MavlinkPassthrough::CommandLong message{};
    message.command = MAV_CMD_DO_SET_MODE;
    message.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    message.param2 = custom_mode;
    message.target_sysid = mavlink_passthrough->get_our_sysid();
    message.target_compid = mavlink_passthrough->get_target_compid();

    //MavlinkPassthrough::CommandLong commandLong = new MavlinkPassthrough::CommandLong(message);
    mavlink_passthrough->send_command_long(message);
    //

    //mavlink_passthrough->queue_message(message);

    //
    //

    std::cout << "Flight mode set to custom mode: " << custom_mode << std::endl;

    // Arm vehicle
    std::cout << "Arming...\n";
    const Action::Result arm_result = action.arm();

    if (arm_result != Action::Result::Success) {
        std::cerr << "Arming failed: " << arm_result << '\n';
        return 1;
    }

    // Take off
    std::cout << "Taking off...\n";
    const Action::Result takeoff_result = action.takeoff();
    if (takeoff_result != Action::Result::Success) {
        std::cerr << "Takeoff failed: " << takeoff_result << '\n';
        return 1;
    }

    // Let it hover for a bit before landing again.
    sleep_for(seconds(10));

    std::cout << "Landing...\n";
    const Action::Result land_result = action.land();
    if (land_result != Action::Result::Success) {
        std::cerr << "Land failed: " << land_result << '\n';
        return 1;
    }

    // Check if vehicle is still in air
    while (telemetry.in_air()) {
        std::cout << "Vehicle is landing...\n";
        sleep_for(seconds(1));
    }
    std::cout << "Landed!\n";

    // We are relying on auto-disarming but let's keep watching the telemetry for a bit longer.
    sleep_for(seconds(3));
    std::cout << "Finished...\n";

    return 0;
}