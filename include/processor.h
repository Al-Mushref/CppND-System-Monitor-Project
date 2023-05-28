#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long prev_total_active_time_ = 0;
  long prev_total_idle_time_ = 0;
};

#endif
