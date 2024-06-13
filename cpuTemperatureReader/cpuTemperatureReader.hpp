#ifndef CPU_TEMPERATURE_READER_HPP
#define CPU_TEMPERATURE_READER_HPP

#include <iostream>
#include <sensors/sensors.h>

class CpuTemperatureReader {
public:
    CpuTemperatureReader();
    ~CpuTemperatureReader();
    void readTemperatures();

private:
    void initializeSensors();
    void cleanupSensors();
};
 
#endif // CPU_TEMPERATURE_READER_H

