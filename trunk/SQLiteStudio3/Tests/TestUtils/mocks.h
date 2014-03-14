#ifndef UNITS_SETUP_H
#define UNITS_SETUP_H

#include "hippomocks.h"

MockRepository& mockRepo();
void deleteMockRepo();

void initMocks();

#endif // UNITS_SETUP_H
