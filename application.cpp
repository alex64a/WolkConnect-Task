/**
 * Copyright 2022 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/persistence/inmemory/InMemoryPersistence.h"
#include "core/utilities/FileSystemUtils.h"
#include "core/utilities/Logger.h"
#include "wolk/WolkBuilder.h"
#include "wolk/WolkSingle.h"
#include "cpuTemperatureReader/cpuTemperatureReader.hpp"
#include "ipAddressReader/ipAddressReader.hpp"

#include <chrono>
#include <csignal>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <thread>

/**
 * This is the place where user input is required for running the example.
 * In here, you can enter the device credentials to successfully identify the device on the platform.
 * And also, the target platform path.
 */
const std::string DEVICE_KEY = "AM";
const std::string DEVICE_PASSWORD = "T6RCB4VIE7";
const std::string PLATFORM_HOST = "integration5.wolkabout.com:1883";
const std::string CA_CERT_PATH = "/INSERT/PATH/TO/YOUR/CA.CRT/FILE";
const std::string FILE_MANAGEMENT_LOCATION = "./files";

/**
 * This is a structure definition that is a collection of all information/feeds the device will have.
 */
struct DeviceData
{
    std::vector<double> temperatures;
    std::string ipAddress;
    std::string logInfo;
};

std::mutex mutex;
std::condition_variable conditionVariable;

class DeviceDataChangeHandler : public wolkabout::connect::FeedUpdateHandler
{
public:
    /**
     * Default constructor that will establish the relationship between the handler and the data.
     *
     * @param deviceData The data object in which  the handler will put the data.
     */
    explicit DeviceDataChangeHandler(DeviceData& deviceData) : m_deviceData(deviceData) {}

    /**
     * This is the overridden method from the `FeedUpdateHandler` interface.
     * This is the method that will receive information about a feed.
     *
     * @param readings The map containing information about updated feeds and their new value(s).
     */
    void handleUpdate(const std::string& deviceKey,
                      const std::map<std::uint64_t, std::vector<wolkabout::Reading>>& readings) override
    {
        // Go through all the timestamps
        for (const auto& pair : readings)
        {
            LOG(DEBUG) << "Received feed information for time: " << pair.first;

            // Take the readings, and apply them
            for (const auto& reading : pair.second)
            {
                LOG(DEBUG) << "Received feed information for reference '" << reading.getReference() << "'.";

                // Lock the mutex
                std::lock_guard<std::mutex> lock{mutex};
                // Check the reference on the readings
                if (reading.getReference() == "LOG_LEVEL")
                    m_deviceData.logInfo = reading.getStringValue();
                wolkabout::LogLevel log = wolkabout::from_string(m_deviceData.logInfo);
                wolkabout::Logger::setLevel(log);
            }

            // Notify the condition variable
            conditionVariable.notify_one();
        }
    }

private:
    // This is where the object containing all information about the device is stored.
    DeviceData& m_deviceData;
};

// A function to return the maximum value of all CPU core temperatures
double getMaximumTemperature(std::vector<double> temperatures)
{
    double maxValue = temperatures[0];
    for (auto itr : temperatures)
    {
        if (temperatures[itr] > maxValue)
            maxValue = temperatures[itr];
    }
    return maxValue;
}

int main(int /* argc */, char** /* argv */)
{
    // object of class CpuTemperatureReader for reading the core temperatures of the CPU
    CpuTemperatureReader temperatureReader;

    // object of class IPAddressReader for reading the IP address
    IPAddressReader ipReader;
    // This is the logger setup. Here you can set up the level of logging you would like enabled.
    wolkabout::Logger::init(wolkabout::LogLevel::DEBUG, wolkabout::Logger::Type::CONSOLE,
                            "/home/amitrovcan/LogInfo/log*.txt");
    // wolkabout::Logger::init(wolkabout::LogLevel::DEBUG, wolkabout::Logger::Type::FILE,
    // "/home/amitrovcan/LogInfo/log*.txt");
    //  wolkabout::Logger::setupConsoleLogger();
    // wolkabout::Logger::setupFileLogger("/home/amitrovcan/LogInfo/log*.txt");

    // Here we create the device that we are presenting as on the platform.
    auto device = wolkabout::Device(DEVICE_KEY, DEVICE_PASSWORD, wolkabout::OutboundDataMode::PUSH);
    auto deviceInfo = DeviceData{{0.0}, "", "INFO"};
    auto deviceInfoHandler = std::make_shared<DeviceDataChangeHandler>(deviceInfo);

    // And here we create the wolk session
    auto wolk = wolkabout::connect::WolkBuilder(device)
                  .host(PLATFORM_HOST)
                  .feedUpdateHandler(deviceInfoHandler)
                  .buildWolkSingle();

    wolk->connect();
    bool running = true;

    // Read the CPU temperature values and the IP address (initial value)
    std::vector<double> temperatures = temperatureReader.readTemperatures();
    std::string ip = ipReader.getIPAddress();
    deviceInfo.temperatures = temperatures;
    deviceInfo.ipAddress = ip;
    deviceInfo.logInfo;

    // Timers for publishing in intervals to the platform and their lambda functions

    wolkabout::Timer timerCpuMax;
    timerCpuMax.run(std::chrono::seconds(60),
                    [&]
                    {
                        temperatures = temperatureReader.readTemperatures();
                        wolk->addReading("CPU_T_core_max", getMaximumTemperature(temperatures));
                        LOG(DEBUG) << "Max CPU core temperature is " << getMaximumTemperature(temperatures);
                        std::cout << "Sending max temperature value at time: "
                                  << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl
                                  << "with value: " << getMaximumTemperature(temperatures) << std::endl;
                        temperatures.clear();
                    });

    wolkabout::Timer timerCpu;
    timerCpu.run(std::chrono::seconds(30),
                 [&]
                 {
                     std::cout << "Sending temperature values at time: "
                               << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl
                               << "with values: " << std::endl
                               << "CPU_T_core1: " << temperatures[0] << std::endl
                               << "CPU_T_core2: " << temperatures[1] << std::endl
                               << "CPU_T_core3: " << temperatures[2] << std::endl
                               << "CPU_T_core4: " << temperatures[3] << std::endl;
                     temperatures.clear();
                     wolk->addReading("CPU_T_core1", temperatures[0]);
                     wolk->addReading("CPU_T_core2", temperatures[1]);
                     wolk->addReading("CPU_T_core3", temperatures[2]);
                     wolk->addReading("CPU_T_core4", temperatures[3]);
                     LOG(DEBUG) << "Published temperatures";
                 });

    wolkabout::Timer timerIp;
    timerIp.run(std::chrono::seconds(30),
                [&]
                {
                    wolk->addReading("IP_ADD", ip);
                    LOG(DEBUG) << "Ip address published" << ip;
                });
    // LogLevel
    wolk->addReading("LOG_LEVEL", deviceInfo.logInfo);

    // And now we will periodically (and endlessly) send a random temperature value.
    while (running)
    {
        std::string newIp = ipReader.getIPAddress();
        temperatures = temperatureReader.readTemperatures();

        // Publish new IP_ADD only if it has changed
        if (deviceInfo.ipAddress != newIp)
        {
            wolk->addReading("IP_ADD", ip);
            wolk->publish();
            ip = newIp;
            deviceInfo.ipAddress = ip;
            LOG(INFO) << "\t IP_ADD changed";
            LOG(INFO) << "NEW IP: " << ip;
        }

        wolk->publish();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
