#ifndef CPU_TEMPERATURE_READER_H
#define CPU_TEMPERATURE_READER_H

#include <iostream>
#include <vector>
#include <sensors/sensors.h>

namespace CpuTemperatureReader
{

std::vector<double> readTemperatures();
void initializeSensors();
void cleanupSensors();
}    // namespace CpuTemperatureReader

#endif    // CPU_TEMPERATURE_READER_H
