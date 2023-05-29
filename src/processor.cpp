#include "processor.h"

#include "linux_parser.h"
#include "system.h"

// Returns the aggregate CPU utilization
float Processor::Utilization() {
  std::vector<std::string> utilization = LinuxParser::CpuUtilization();

  float user = std::stof(utilization[0]);
  float nice = std::stof(utilization[1]);
  float system = std::stof(utilization[2]);
  float idle = std::stof(utilization[3]);
  float iowait = std::stof(utilization[4]);
  float irq = std::stof(utilization[5]);
  float softirq = std::stof(utilization[6]);
  float steal = std::stof(utilization[7]);

  float idle_time = idle + iowait;
  float non_idle_time = user + nice + system + irq + softirq + steal;
  float total_time = idle_time + non_idle_time;

  float utilization_percentage = (total_time - idle_time) / total_time;

  return utilization_percentage;
}
