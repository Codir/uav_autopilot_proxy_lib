#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

using namespace mavsdk;

class LinkAutopilotInterface {
public:
    LinkAutopilotInterface();
    ~LinkAutopilotInterface();

    bool connect(std::string connection_url);
    void setModeGuidedNoGPS();
    void armThrottle();
    void takeoff(float altitude);

private:
    mavsdk::Mavsdk mavsdk;
    std::shared_ptr<mavsdk::System> system;
    mavsdk::Telemetry* telemetry;
    mavsdk::Action* action;

    //void sendMAVLinkCommand(uint16_t command, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0, float param5 = 0, float param6 = 0, float param7 = 0);
};
