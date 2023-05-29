#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "process.h"

using std::getline;
using std::ifstream;
using std::istringstream;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Reads and returns the OS
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Reads and returns the linux kernel
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Reads and returns the proccesses ids
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Reads and returns the system memory utilization
float LinuxParser::MemoryUtilization() {
  ifstream mem_info_file(kProcDirectory + kMeminfoFilename);
  string line;
  float total_memory = 0.0f;
  float free_memory = 0.0f;
  float buffers = 0.0f;

  while (getline(mem_info_file, line)) {
    istringstream iss(line);
    string key;
    float value;

    if (iss >> key >> value) {
      if (key == "MemTotal:") {
        total_memory = value;
      } else if (key == "MemFree:") {
        free_memory = value;
      } else if (key == "Bufers:") {
        buffers = value;
      }
    }
  }
  return (1.0 - (free_memory / (total_memory - buffers)));
}

// Reads and returns the system uptime
long int LinuxParser::UpTime() {
  ifstream uptime_file(kProcDirectory + kUptimeFilename);
  string line;
  if (!uptime_file.is_open()) {
    throw std::invalid_argument("could not open uptime file");
  }
  getline(uptime_file, line);
  istringstream ss(line);
  long uptime = 0;
  ss >> uptime;
  return uptime;
}

// Reads and returns the number of jiffies for the system
long LinuxParser::Jiffies() {
  ifstream stat_file(kProcDirectory + kStatFilename);
  string line;
  getline(stat_file, line);
  istringstream iss(line);
  string cpuLabel;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

  // EXTRACT CPU UTILIZATION VALUES
  iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >>
      softirq >> steal >> guest >> guest_nice;

  // CALCULATE TOTAL JIFFIES
  long totalJiffies = user + nice + system + idle + iowait + irq + softirq +
                      steal + guest + guest_nice;

  return totalJiffies;
}

// Reads and returns the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  ifstream stat_file(kProcDirectory + std::to_string(pid) + kStatFilename);
  string line;
  if (getline(stat_file, line)) {
    istringstream linestream(line);
    std::istream_iterator<string> line_iterator(linestream), end_iterator;
    std::vector<string> stat_fields(line_iterator, end_iterator);

    long utime = std::stol(stat_fields[13]);
    long stime = std::stol(stat_fields[14]);
    long cutime = std::stol(stat_fields[15]);
    long cstime = std::stol(stat_fields[16]);

    long active_jiffies = utime + stime + cutime + cstime;
    return active_jiffies;
  }

  return 0;
}

// Reads and returns the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long total_jiffies = Jiffies();
  long idle_jiffies = IdleJiffies();
  long active_jiffies = total_jiffies - idle_jiffies;
  return active_jiffies;
}

// Reads and returns the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  ifstream stat_file(kProcDirectory + kStatFilename);
  string line;
  string cpu_label;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

  if (!stat_file.is_open()) {
    throw std::runtime_error("cannot open stat file");
  }
  getline(stat_file, line);
  istringstream linestream(line);
  // EXTRACT IDLE JIFFIES
  linestream >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >>
      softirq >> steal >> guest >> guest_nice;
  // CALCULATE IDLE JIFFIES
  long idleJiffies = idle + iowait;
  return idleJiffies;
}

// Reads and returns CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  ifstream stat_file(kProcDirectory + kStatFilename);
  string line;
  vector<string> cpu_utilization;

  if (!stat_file.is_open()) {
    throw std::runtime_error("cannot open stat file");
  }
  getline(stat_file, line);
  istringstream linestream(line);
  string cpu_label;
  linestream >> cpu_label;

  if (cpu_label == "cpu") {
    string value;
    while (linestream >> value) {
      cpu_utilization.emplace_back(value);
    }
  }

  return cpu_utilization;
}

// Reads and returns the total number of processes
int LinuxParser::TotalProcesses() {
  ifstream memory_info_file(kProcDirectory + kStatFilename);
  string line;
  int processes = 0;
  while (getline(memory_info_file, line)) {
    istringstream ss(line);
    string key;
    int value;
    if (ss >> key >> value) {
      if (key == "processes") {
        processes = value;
        break;
      }
    }
  }
  return processes;
}

// Reads and returns the number of running processes
int LinuxParser::RunningProcesses() {
  ifstream memory_info_file(kProcDirectory + kStatFilename);
  string line;
  int running_procs = 0;
  while (getline(memory_info_file, line)) {
    istringstream ss(line);
    string key;
    int value;
    if (ss >> key >> value) {
      if (key == "procs_running") {
        running_procs = value;
        break;
      }
    }
  }
  return running_procs;
}

// Reads and returns the command associated with a process
string LinuxParser::Command(int pid) { return Process(pid).Command(); }

// Reads and returns the memory used by a process
string LinuxParser::Ram(int pid) { return Process(pid).Ram(); }

// Reads and returns the user ID associated with a process
string LinuxParser::Uid(int pid) { return Process(pid).Uid(); }

// Reads and returns the user associated with a process
string LinuxParser::User(int pid) { return Process(pid).User(); }

// Reads and returns the uptime of a process
long LinuxParser::UpTime(int pid) { return Process(pid).UpTime(); }
