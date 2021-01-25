#ifndef ___STATUSCODE_H___
#define ___STATUSCODE_H___

namespace AVSI {

    typedef enum
    {
        STATUS_NONE = 0,
        STATUS_OK,
        STATUS_ERR,
        STATUS_RET
    } StatusCode;

    void setStatus(StatusCode s);
    StatusCode getStatus(void);
} // namespace AVSI

#endif