#include "mavsdk_interface.h"
#include <cstdint>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::this_thread;
using namespace std::chrono;

LinkAutopilotInterface::LinkAutopilotInterface()     {
}

LinkAutopilotInterface::~LinkAutopilotInterface() {
}

bool LinkAutopilotInterface::connect(std::string connection_url) {
    
    mavsdk::Mavsdk mavsdk{mavsdk::Mavsdk::Configuration{mavsdk::Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Failed to connect: " << ConnectionResult(connection_result) << std::endl;
        return false;
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system) {
        std::cerr << "Timed out waiting for system\n";
        return 1;
    }

    telemetry = new Telemetry{system.value()};
    action = new Action{system.value()};

    // Ожидаем готовности дрона
    while (!telemetry->health_all_ok()) {
        std::cout << "Waiting for drone to be ready..." << std::endl;
        sleep_for(seconds(1));
    }

    std::cout << "Drone is ready!" << std::endl;
    return true;
}

void LinkAutopilotInterface::setModeGuidedNoGPS() {
    // Используем MAVLink для отправки команды SET_MODE
    mavsdk::MavlinkPassthrough mavlink_passthrough{system};
    mavlink_message_t message;
    uint8_t base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    uint32_t custom_mode = 4; // GUIDED_NO_GPS для ArduPilot

    mavlink_msg_set_mode_pack(255, 0, &message, system->get_system_id(), base_mode, custom_mode);
    mavlink_passthrough.send_message(message);

    std::cout << "Switched to GUIDED_NO_GPS mode" << std::endl;
}

void LinkAutopilotInterface::armThrottle() {
    Action::Result arm_result = action->arm();
    if (arm_result != Action::Result::Success) {
        std::cerr << "Arm failed: " << arm_result << std::endl;
    } else {
        std::cout << "Drone armed" << std::endl;
    }
}

void LinkAutopilotInterface::takeoff(float altitude) {
    Action::Result takeoff_result = action->takeoff();
    if (takeoff_result != Action::Result::Success) {
        std::cerr << "Takeoff failed: " << takeoff_result << std::endl;
    } else {
        std::cout << "Taking off to altitude: " << altitude << " meters" << std::endl;
    }

    // Ждем, пока дрон не достигнет желаемой высоты
    sleep_for(seconds(10));
}

/*void LinkAutopilotInterface::sendMAVLinkCommand(uint16_t command, float param1, float param2, float param3, float param4, float param5, float param6, float param7) {
    mavsdk::MavlinkPassthrough mavlink_passthrough{system};
    mavlink_message_t message;

    mavlink_msg_command_long_pack(
        255, 0, &message, system->get_system_id(), system->get_component_id(),
        command, 0, param1, param2, param3, param4, param5, param6, param7);

    mavlink_passthrough.send_message(message);
}*/
