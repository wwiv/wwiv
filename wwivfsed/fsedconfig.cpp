/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*                   Copyright (C)2020, WWIV Software Services            */
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
#include "wwivfsed/fsedconfig.h"

#include "core/command_line.h"
#include "core/inifile.h"
#include "core/log.h"
#include "core/version.h"
#include "common/null_remote_io.h"
#include "common/remote_socket_io.h"
#include "local_io/null_local_io.h"
// isatty
#ifdef _WIN32
#include "local_io/local_io_win32.h"
#include <io.h>
#else
#include "local_io/local_io_curses.h"
#include "localui/curses_io.h"
#include <unistd.h>
#endif
using namespace wwiv::common;
using namespace wwiv::core;
using namespace wwiv::strings;
using namespace wwiv::sdk;

namespace wwiv::wwivfsed {


FsedConfig::FsedConfig(const CommandLine& cmdline) : root_(cmdline.program_path().parent_path()) {
  auto path = FilePath(root_, "wwivfsed.ini");
  IniFile ini(path, {"WWIVFSED"});
  if (ini.IsOpen()) {
    LOG(ERROR) << "Unable to read wwivfsed.ini.";
  }

  local_ = cmdline.barg("local");
  pause_ = cmdline.barg("pause");
  socket_handle_ = cmdline.iarg("socket_handle");
  if (!cmdline.remaining().empty()) {
    file_path_ = cmdline.remaining().front();
  }

}

LocalIO* FsedConfig::CreateLocalIO() {
  if (local_ && !isatty(fileno(stdin))) {
    LOG(ERROR) << "WTF - No isatty?";
  }
  if (local_) {
#if defined(_WIN32)
    return new Win32ConsoleIO();
#else
    if (local_) {
      CursesIO::Init(fmt::sprintf("WWIVfsed %s", full_version()));
      return new CursesLocalIO(curses_out->GetMaxY());
    }
#endif
  }
  return new NullLocalIO();
}

wwiv::common::RemoteIO* FsedConfig::CreateRemoteIO() {
  if (local_) {
    return new NullRemoteIO();
  }
  return new RemoteSocketIO(socket_handle_, false);
}

std::filesystem::path FsedConfig::help_path() const { return FilePath(root_, help_path_); }

std::filesystem::path FsedConfig::file_path() const {
  if (!file_path_.empty()) {
    return file_path_;
  }

  return (bbs_type() == FsedConfig::bbs_type::wwiv) ? "input.msg" : "msgtmp";
}


}

// { IniFile ini("net.ini", {"NETWORK"}); }