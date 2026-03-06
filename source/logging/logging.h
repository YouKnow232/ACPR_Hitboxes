#include <string>
#include "gearLoader/gearLoader.hpp"

namespace ACPRHitboxes {
    class Logger {
        public:
        Logger(GearLoader::Api* api) : _api(api) {}
        int Log(GearLoader::LogLevel level, std::string msg) {
            return _api->Log(level, msg.c_str());
        }
        
        private:
        GearLoader::Api* _api;
    };
    
    Logger& GetLogger();
    void SetLogger(GearLoader::Api* api);
}