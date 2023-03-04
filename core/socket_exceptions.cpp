/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2015-2022, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/**************************************************************************/
#include "core/socket_exceptions.h"
#include "core/log.h"
#include "core/strings.h"
#include "fmt/format.h"
#include <stdexcept>

namespace wwiv::core {

connection_error::connection_error(const std::string& host, int port)
  : socket_error(fmt::format("Error connecting to: {}:{}", host, port)) {
  LOG(ERROR) << "connection_error: " << host << ":" << port;
}

socket_error::socket_error(const std::string& message)
  : std::runtime_error(message) {
  // Don't log this since we *often* hit a timeout since we use nonblocking
  // sockets and that's OK.
  VLOG(4) << "socket_error: " << message;
}


} // namespace wwiv
