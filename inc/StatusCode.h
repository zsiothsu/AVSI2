#ifndef ___STATUSCODE_H___
#define ___STATUSCODE_H___

#include <stdint.h>

namespace AVSI {
    
    #define Status_none             0x00
    #define Status_ret              0x01
    #define Status_inFunction       0x02
    #define StatusCode              uint16_t

    void clearStatus(uint16_t s);
    bool getStatus(uint16_t s);
    void setStatus(uint16_t s);
} // namespace AVSI

#endif