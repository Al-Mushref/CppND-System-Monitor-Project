#include "processor.h"

#include "linux_parser.h"
#include "system.h"

// Returns the aggregate CPU utilization
float Processor::Utilization() {
  float total_time = LinuxParser::Jiffies();
  float active_time = LinuxParser::ActiveJiffies();
  return active_time / total_time;
}
