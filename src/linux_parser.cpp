#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "process.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Reads and returns the OS
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
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
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
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
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Reads and returns the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream mem_info_file(kProcDirectory + kMeminfoFilename);
  std::string line;
  float total_memory = 0.0f;
  float free_memory = 0.0f;
  float buffers = 0.0f;

  while (std::getline(mem_info_file, line)) {
    std::istringstream iss(line);
    std::string key;
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
  std::ifstream uptime_file(kProcDirectory + kUptimeFilename);
  string line;
  if (!uptime_file.is_open()) {
    throw std::invalid_argument("could not open uptime file");
  }
  getline(uptime_file, line);
  std::istringstream ss(line);
  long uptime = 0;
  ss >> uptime;
  return uptime;
}

// Reads and returns the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::ifstream statFile(kProcDirectory + kStatFilename);
  std::string line;
  std::getline(statFile, line);
  std::istringstream iss(line);
  std::string cpuLabel;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guestNice;

  // EXTRACT CPU UTILIZATION VALUES
  iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >>
      softirq >> steal >> guest >> guestNice;

  // CALCULATE TOTAL JIFFIES
  long totalJiffies = user + nice + system + idle + iowait + irq + softirq +
                      steal + guest + guestNice;

  return totalJiffies;
}

// Reads and returns the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream statFile(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line;
  if (std::getline(statFile, line)) {
    std::istringstream lineStream(line);
    std::istream_iterator<std::string> lineIterator(lineStream), endIterator;
    std::vector<std::string> statFields(lineIterator, endIterator);

    long utime = std::stol(statFields[13]);
    long stime = std::stol(statFields[14]);
    long cutime = std::stol(statFields[15]);
    long cstime = std::stol(statFields[16]);

    long activeJiffies = utime + stime + cutime + cstime;
    return activeJiffies;
  }

  return 0;
}

// Reads and returns the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long totalJiffies = Jiffies();
  long idleJiffies = IdleJiffies();
  long activeJiffies = totalJiffies - idleJiffies;
  return activeJiffies;
}

// Reads and returns the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::ifstream statFile(kProcDirectory + kStatFilename);
  std::string line;
  std::string cpuLabel;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guestNice;

  if (statFile.is_open()) {
    std::getline(statFile, line);
    std::istringstream lineStream(line);
    // EXTRACT IDLE JIFFIES
    lineStream >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guestNice;
  }
  // CALCULATE IDLE JIFFIES
  long idleJiffies = idle + iowait;
  return idleJiffies;
}

// Reads and returns CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  string line;
  vector<string> cpuUtilization;

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    string cpuLabel;
    linestream >> cpuLabel;

    if (cpuLabel == "cpu") {
      string value;
      while (linestream >> value) {
        cpuUtilization.push_back(value);
      }
    }
  }

  return cpuUtilization;
}

// Reads and returns the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream memory_info_file(kProcDirectory + kStatFilename);
  string line;
  int processes = 0;
  while (std::getline(memory_info_file, line)) {
    std::istringstream ss(line);
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
  std::ifstream memory_info_file(kProcDirectory + kStatFilename);
  string line;
  int running_procs = 0;
  while (std::getline(memory_info_file, line)) {
    std::istringstream ss(line);
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
