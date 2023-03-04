/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2022, WWIV Software Services            */
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
/*                                                                        */
/**************************************************************************/
#include "core/semaphore_file.h"

#include "stl.h"
#include "core/file.h"
#include "core/log.h"
#include "core/os.h"
#include <cerrno>
#include <fcntl.h>
#include <string>
#include <utility>

#ifdef _WIN32
//#include <direct.h>
#include <io.h>
//#include <share.h>

#else // _WIN32
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#endif  // _WIN32

using std::chrono::milliseconds;
using namespace wwiv::os;

namespace wwiv::core {

#ifndef _WIN32
#ifndef O_TEMPORARY
#define O_TEMPORARY 0
#endif  // O_TEMPORARY
#endif  // _WIN32

// static 
SemaphoreFile SemaphoreFile::try_acquire(const std::filesystem::path& filepath,
                                         const std::string& text,
                                         std::chrono::duration<double> timeout) {
  VLOG(2) << "SemaphoreFile::try_acquire: '" << filepath << "'";
  const auto mode = O_CREAT | O_EXCL | O_TEMPORARY | O_RDWR;
  const auto pmode = S_IREAD | S_IWRITE;
  auto step = timeout / 10;
  if (step > std::chrono::seconds(1)) {
    step = std::chrono::seconds(1);
  }
  const auto start = std::chrono::steady_clock::now();
  const auto end = start + timeout;
  while (true) {
    const auto fn = filepath.string();
    if (const auto fd = open(fn.c_str(), mode, pmode); fd >= 0) {
      if (const auto written = write(fd, text.c_str(), wwiv::stl::size_int(text));
          written != wwiv::stl::ssize(text)) {
        LOG(WARNING) << "Short write on Semaphore file: " << written << "; vs: " << text.size();
      }
      return {filepath, fd};
    }
    if (std::chrono::steady_clock::now() > end) {
      VLOG(1) << "FAILED: SemaphoreFile::try_acquire: '" << filepath << "'";
      throw semaphore_not_acquired(filepath);
    }
    sleep_for(step);
  }
}

// static 
SemaphoreFile SemaphoreFile::acquire(const std::filesystem::path& filepath,
                                     const std::string& text) {
  return try_acquire(filepath, text, std::chrono::duration<double>::max());
}

SemaphoreFile::SemaphoreFile(std::filesystem::path path, int fd)
  : path_(std::move(path)), fd_(fd) {
}

SemaphoreFile::~SemaphoreFile() {
  VLOG(1) << "~SemaphoreFile(): " << path_ << "; fd: " << fd_;
  if (fd_ < 0) {
    LOG(ERROR) << "Skipping closing since file already closed: " << path_;
    return;
  }
  if (close(fd_) == -1) {
    LOG(ERROR) << "Failed to close file: " << path_ << "; error: " << errno;
  }
  fd_ = -1;
#ifndef _WIN32
  // Since we don't have O_TEMPORARY outside of WIN32, we unlink the file
  // which will delete it once the last file handle is closed.
  if (!File::Remove(path_, true)) {
    LOG(ERROR) << "Failed to unlink file: " << path_ << "; error: " << errno;
  }
#endif  // _WIN32
}


} // namespace wwiv
