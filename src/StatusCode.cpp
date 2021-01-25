#include "../inc/StatusCode.h"

namespace AVSI {
    StatusCode RUNNING_STATUS = NONE;

    void setStatus(StatusCode s)
    {
        RUNNING_STATUS = s;
    }

    StatusCode getStatus(void)
    {
        StatusCode ret = RUNNING_STATUS;
        RUNNING_STATUS = NONE;
        return ret;
    }

} // namespace AVSI