#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "processor.h"
#include "system.h"

using std::ifstream;
using std::istringstream;
using std::string;
using std::to_string;
using std::vector;

// Returns this process's ID
int Process::Pid() { return pid_; }

// Returns this process's CPU utilization
float Process::CpuUtilization() {
  float total_time = LinuxParser::ActiveJiffies(pid_);
  return 100 * (total_time / sysconf(_SC_CLK_TCK)) / UpTime();
}

// Returns the command that generated this process
string Process::Command() {
  ifstream cmd_file(LinuxParser::kProcDirectory + to_string(pid_) +
                    LinuxParser::kCmdlineFilename);

  if (!cmd_file.is_open()) {
    throw std::runtime_error("cannot open command file file");
  }
  string cmd_line;
  getline(cmd_file, cmd_line);
  return cmd_line;
}

// Returns this process's memory utilization
string Process::Ram() {
  ifstream status_file(LinuxParser::kProcDirectory + to_string(pid_) +
                       LinuxParser::kStatusFilename);
  if (!status_file.is_open()) {
    throw std::runtime_error("cannot open status file");
  }
  string line;
  string ram;
  while (getline(status_file, line)) {
    istringstream iss(line);
    string key;
    long value;
    if (iss >> key >> value) {
      if (key == "VmSize:") {
        ram = to_string(value / 1024);
      }
    }
  }
  return ram;
}

// Returns the user (name) that generated this process
string Process::User() {
  // FIND UID
  ifstream status_file(LinuxParser::kProcDirectory + to_string(pid_) +
                       LinuxParser::kStatusFilename);
  if (!status_file.is_open()) {
    throw std::runtime_error("cannot open status file");
  }
  string line;
  while (getline(status_file, line)) {
    istringstream iss(line);
    string key;
    long value;
    if (iss >> key >> value) {
      if (key == "Uid:") {
        uid_ = to_string(value);
        break;
      }
    }
  }
  // FIND USER CORRESPONDING TO UID
  string user;
  ifstream pass_file(LinuxParser::kPasswordPath);
  if (!pass_file.is_open()) {
    throw std::runtime_error("cannot open password path");
  }
  string ps_line;
  while (getline(pass_file, ps_line)) {
    istringstream iss(ps_line);
    std::string username, x, uid_str;
    if (getline(iss, username, ':') && getline(iss, x, ':') &&
        getline(iss, uid_str, ':')) {
      try {
        if (uid_str == uid_) {
          return username;
        }
      } catch (...) {
        // IGNORE TO CHECK NEXT LINE
      }
    }
  }
  return "";
}

// Returns the age of this process (in seconds)
long int Process::UpTime() {
  ifstream stat_file(LinuxParser::kProcDirectory + to_string(pid_) +
                     LinuxParser::kStatFilename);
  if (!stat_file.is_open()) {
    throw std::runtime_error("Cannot open stat file");
  }

  std::string line;
  std::getline(stat_file, line);
  istringstream iss(line);

  // POPULATE THE VECTOR WITH ALL VALUES FROM ISS FROM START TO END
  std::vector<std::string> stat_values(std::istream_iterator<std::string>{iss},
                                       std::istream_iterator<std::string>());

  if (stat_values.size() <= 21) {
    throw std::runtime_error("Invalid stat file format");
  }

  // START TIME VALUE IS THE 22nd IN THE FILE
  long int starttime = std::stol(stat_values[21]);
  long int sys_uptime = LinuxParser::UpTime();
  long int clock_ticks = sysconf(_SC_CLK_TCK);

  // WE DIVIDE starttime by clock_ticks TO GET TIME IN SECONDS
  return sys_uptime - (starttime / clock_ticks);
}

bool Process::operator<(Process const& a) const { return pid_ < a.pid_; }