#include "logging.h"

namespace ACPRHitboxes {

    static GearLoader::Api* _api;
    
    Logger& StaticLogger(GearLoader::Api* api) {
        static Logger logger(api);
        return logger;
    }
    Logger& GetLogger() {
        return StaticLogger(nullptr);
    }
    void SetLogger(GearLoader::Api* api) {
        StaticLogger(api);
    }
}
