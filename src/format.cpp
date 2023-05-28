#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::setfill;
using std::setw;
using std::string;
using std::stringstream;

string Format::ElapsedTime(long seconds) {
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;

  // setw() used to ensure that the length stays fixed = 2
  // setfill() allows leading zeros
  stringstream ss;
  ss << setw(2) << setfill('0') << hours << ":";
  ss << setw(2) << setfill('0') << minutes << ":";
  ss << setw(2) << setfill('0') << secs;

  return ss.str();
}