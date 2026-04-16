#include "dataStore.h"
#include "baseMod/baseMod_c.h"
#include "baseMod/baseMod.hpp"

// Store the BaseMod api here. Retrive the wrapper class with a parameterless call later.

static BaseMod::Api api;

BaseMod::Api& SetBMApi(const BaseMod_Api* cApi) {
    api = BaseMod::Api(cApi);
    return api;
}

BaseMod::Api& BMApi() { return api; }
