#include "processor.h"

#include "linux_parser.h"
#include "system.h"

// Returns the aggregate CPU utilization
float Processor::Utilization() {
  return LinuxParser::ActiveJiffies() / LinuxParser::Jiffies();
}
