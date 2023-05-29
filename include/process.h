#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid) : pid_{pid} {};
  Process() = default;
  std::string Uid() const { return uid_; };
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process& a);

 private:
  int pid_ = 0;
  std::string uid_ = "";
};

#endif