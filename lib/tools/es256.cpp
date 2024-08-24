#include "es256.h"

Es256::Es256() { }

Es256 *Es256::getInstance()
{
    static Es256 instance;
    return &instance;
}
