#ifndef CPU_TEMPERATURE_READER_H
#define CPU_TEMPERATURE_READER_H

#include <iostream>
#include <vector>
#include <sensors/sensors.h>

class CpuTemperatureReader {
public:
    CpuTemperatureReader();
    ~CpuTemperatureReader();
    std::vector<double> readTemperatures();

private:
    void initializeSensors();
    void cleanupSensors();
};

#endif // CPU_TEMPERATURE_READER_H

