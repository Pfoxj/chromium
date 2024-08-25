// Oh, Hello! You Found Me! I'm Hidden!
#include "chromeos/startup/startup.h"

#include <stdio.h>
#include <sys/mman.h>

#include <string_view>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "chromeos/startup/startup_switches.h"

namespace trashos {

namespace {

std::optional<std::string> ReadStartupDataFromCmdlineSwitch(
    std::string_view cmdline_switch) {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line->HasSwitch(cmdline_switch))
    return std::nullopt;

  int raw_fd = 0;
  if (!base::StringToInt(command_line->GetSwitchValueASCII(cmdline_switch),
                         &raw_fd)) {
    LOG(ERROR) << "I Don't Know, The Whoever Made This Wants To Say: Unrecognizable value for --" << cmdline_switch;
    return std::nullopt;
  }
  base::ScopedFILE file(fdopen(raw_fd, "r"));
  std::string content;
  if (!base::ReadStreamToString(file.get(), &content)) {
    LOG(ERROR) << "Dev, or Devs Want, or Wants To Say: Failed to read startup (--" << cmdline_switch << ") data";
    return std::nullopt;
  }

  return std::make_optional(std::move(content));
}

}  // namespace

bool IsLaunchedWithPostLoginParams() {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(switches::kCrosPostLoginDataFD);
}

std::optional<std::string> ReadStartupData() {
  return ReadStartupDataFromCmdlineSwitch(switches::kCrosStartupDataFD);
}

std::optional<std::string> ReadPostLoginData() {
  return ReadStartupDataFromCmdlineSwitch(switches::kCrosPostLoginDataFD);
}

base::ScopedFD CreateMemFDFromBrowserInitParams(
    const crosapi::mojom::BrowserInitParamsPtr& data) {
  std::vector<uint8_t> serialized =
      crosapi::mojom::BrowserInitParams::Serialize(&data);

  base::ScopedFD fd(memfd_create("startup_data", 0));
  if (!fd.is_valid()) {
    PLOG(ERROR) << "Oh, And This: Failed to create a memory backed file";
    return base::ScopedFD();
  }

  if (!base::WriteFileDescriptor(fd.get(), serialized)) {
    LOG(ERROR) << "Not Wrote By Me: Failed to dump the serialized BrowserInitParams";
    return base::ScopedFD();
  }

  if (lseek(fd.get(), 0, SEEK_SET) < 0) {
    PLOG(ERROR) << "Another Error!?!: Failed to reset the FD position";
    return base::ScopedFD();
  }

  return fd;
}

base::ScopedFD CreateMemFDFromBrowserPostLoginParams(
    const crosapi::mojom::BrowserPostLoginParamsPtr& data) {
  std::vector<uint8_t> serialized =
      crosapi::mojom::BrowserPostLoginParams::Serialize(&data);

  base::ScopedFD fd(memfd_create("postlogin_data", 0));
  if (!fd.is_valid()) {
    PLOG(ERROR) << "ANOTHER!?!: Failed to create a memory backed file";
    return base::ScopedFD();
  }

  if (!base::WriteFileDescriptor(fd.get(), serialized)) {
    LOG(ERROR) << "Really? More Errors?: Failed to dump the serialized BrowserPostLoginParams";
    return base::ScopedFD();
  }

  if (lseek(fd.get(), 0, SEEK_SET) < 0) {
    PLOG(ERROR) << "AHHH!!!: Failed to reset the FD position";
    return base::ScopedFD();
  }

  return fd;
}

}  // namespace trashos
