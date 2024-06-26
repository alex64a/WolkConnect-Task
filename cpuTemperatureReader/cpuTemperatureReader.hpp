#ifndef CPU_TEMPERATURE_READER_H
#define CPU_TEMPERATURE_READER_H

#include <iostream>
#include <vector>
#include <sensors/sensors.h>
/**
 * @brief Gets the temperature of cpu cores of local machine
 * @return std::vector<double> of temperature of cores (4 cores)
 */
namespace CpuTemperatureReader
{

std::vector<double> readTemperatures();
void initializeSensors();
void cleanupSensors();
}    // namespace CpuTemperatureReader

#endif    // CPU_TEMPERATURE_READER_H
