#include "linux_parser.h"
#include "ncurses_display.h"
#include "system.h"
using namespace std;
#include <iostream>
int main() {
  System system;
  NCursesDisplay::Display(system);
}
