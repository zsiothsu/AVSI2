#include "../inc/StatusCode.h"

namespace AVSI {
    uint16_t RUNNING_STATUS = 0;

    void clearStatus(uint16_t s)
    {
        RUNNING_STATUS &= ~s;
    }

    void setStatus(uint16_t s)
    {
        RUNNING_STATUS |= s;
    }

    bool getStatus(uint16_t s)
    {
        return RUNNING_STATUS & s;
    }

} // namespace AVSI