#ifndef PROCESS_ROUTER_INITIALIZER_H
#define PROCESS_ROUTER_INITIALIZER_H
/**
 * @file process/RouterInitializer.h
 * @author Ian Yang
 * @date Created <2010-05-31 17:57:33>
 */

#include <util/driver/Router.h>

namespace fibp {
void initializeDriverRouter(::izenelib::driver::Router& router, bool enableTest = false);

} // namespace 

#endif // PROCESS_ROUTER_INITIALIZER_H
