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
#include "json/single_include/nlohmann/json.hpp"
#include <fstream>
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
 * timers and log level initialization
 * Change the @param LOG_LEVEL to info, warn, error, trace or off
 * Change the @param LOG_TYPE to CONSOLE, FILE or BUFFER
 * Change the @oaram FILE_PATH to the path where you want to save the log file
 */

const wolkabout::LogLevel LOG_LEVEL = wolkabout::LogLevel::DEBUG;
const std::string LOG_LEVEL_STRING = "DEBUG";
const wolkabout::Logger::Type LOG_TYPE = wolkabout::Logger::Type::CONSOLE | wolkabout::Logger::Type::FILE;
const std::string FILE_PATH = "/log.txt";
const int CPU_TIMER = 30;
const int CPU_TIMER_MAX = 60;
const int IP_TIMER = 30;

/**
 * This is the place where user input is required for running the example.
 * In here, you can enter the device credentials to successfully identify the device on the platform.
 * And also, the target platform path.
 */
std::string DEVICE_KEY = "";
std::string DEVICE_PASSWORD = "";
std::string PLATFORM_HOST = "";
std::string CA_CERT_PATH = "";
std::string FILE_MANAGEMENT_LOCATION = "";

/**This is a string variable to store the path of the json file
 * that is parsed as a command line argument from the user
 */
std::string pathToJsonFile = "";

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

bool isValidLog(const std::string& logInfo)
{
    LOG(INFO) << "Received value for Log Level \"" << logInfo << "\"";
    if (logInfo == "INFO" || logInfo == "DEBUG" || logInfo == "WARN" || logInfo == "ERROR" || logInfo == "TRACE" ||
        logInfo == "OFF" || logInfo == "info" || logInfo == "debug" || logInfo == "warn" || logInfo == "error" ||
        logInfo == "trace" || logInfo == "off")
    {
        LOG(DEBUG) << "Log Level value valid!";
        return true;
    }
    LOG(INFO) << "Unsupported log entry, please enter: INFO, DEBUG, WARN, ERROR, TRACE OR OFF";
    return false;
}

bool isValidJson(const nlohmann::json& jsonData)
{
    const std::string requiredKeys[] = {"DEVICE_KEY", "DEVICE_PASSWORD", "PLATFORM_HOST", "CA_CERT_PATH",
                                        "FILE_MANAGEMENT_LOCATION"};

    for (const auto& key : requiredKeys)
    {
        if (!jsonData.contains(key))
        {
            LOG(DEBUG) << "Invalid json reading, please fill all the required fields <DEVICE_KEY>, <DEVICE_PQSSWORD>, "
                          "<PLATFORM_HOST>, <CA_CERT_PATH>, <FILE_MANAGEMENT_LOCATION>";
            return false;
        }
    }

    LOG(DEBUG) << "Valid json document";
    return true;
}

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
                {
                    if (isValidLog(reading.getStringValue()))
                    {
                        m_deviceData.logInfo = reading.getStringValue();

                        wolkabout::LogLevel log = wolkabout::from_string(m_deviceData.logInfo);
                        wolkabout::Logger::getInstance().setLevel(log);
                        LOG(DEBUG) << "LogLevel changed to: " << m_deviceData.logInfo;
                    }
                }
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

int main(int argc, char** argv)
{
    // This is the logger setup. Here you can set up the level of logging you would like enabled.
    wolkabout::Logger::init(LOG_LEVEL, LOG_TYPE, FILE_PATH);
    // check if the command line passes two arguments (./ELabTest and <PATH_TO_JSON_FILE>)
    if (argc != 2)
    {
        LOG(DEBUG) << "Please run with ./ELabTest <PATH_TO_JSON_FILE>";
        std::cout << "Please run with ./ELabTest <PATH_TO_JSON_FILE>" << std::endl;
        return -1;
    }
    // Command line argument
    pathToJsonFile = argv[1];

    if (pathToJsonFile == "")
    {
        LOG(DEBUG) << "No path to json file provided, user inputed empty string";
        std::cout << "No path to json file provided, user inputed empty string";
        return -1;
    }

    // Json file to store the device data parameters (DEVICE_KEY, DEVICE_PASSWORD etc.)
    std::ifstream jsonFileStream(pathToJsonFile);
    if (!jsonFileStream.is_open())
    {
        throw std::runtime_error("Unable to open file: " + pathToJsonFile);
    }
    nlohmann::json jsonData;
    jsonFileStream >> jsonData;
    if (isValidJson(jsonData))
    {
        DEVICE_KEY = jsonData.at("DEVICE_KEY").get<std::string>();
        DEVICE_PASSWORD = jsonData.at("DEVICE_PASSWORD").get<std::string>();
        PLATFORM_HOST = jsonData.at("PLATFORM_HOST").get<std::string>();
        CA_CERT_PATH = jsonData.at("CA_CERT_PATH").get<std::string>();
        FILE_MANAGEMENT_LOCATION = jsonData.at("FILE_MANAGEMENT_LOCATION").get<std::string>();

        std::cout << "Device key: " << DEVICE_KEY << std::endl;
        std::cout << "Device password: " << DEVICE_PASSWORD << std::endl;
        std::cout << "Platform host: " << PLATFORM_HOST << std::endl;
        std::cout << "CA cert path: " << CA_CERT_PATH << std::endl;
        std::cout << "File management location: " << FILE_MANAGEMENT_LOCATION << std::endl;
    }

    auto device = wolkabout::Device(DEVICE_KEY, DEVICE_PASSWORD, wolkabout::OutboundDataMode::PUSH);
    auto deviceInfo = DeviceData{
      {0.0},
      "",
      LOG_LEVEL_STRING,
    };
    deviceInfo.logInfo = LOG_LEVEL_STRING;
    auto deviceInfoHandler = std::make_shared<DeviceDataChangeHandler>(deviceInfo);

    // And here we create the wolk session
    auto wolk = wolkabout::connect::WolkBuilder(device)
                  .host(PLATFORM_HOST)
                  .feedUpdateHandler(deviceInfoHandler)
                  .buildWolkSingle();
    wolk->connect();
    bool running = true;

    std::vector<double> temperatures;
    std::vector<double> temperaturesMax;
    std::string ip = IPAddressReader::retrieveIPAddress();
    deviceInfo.temperatures = temperatures;
    deviceInfo.ipAddress = ip;

    // Timer for publishing in intervals to the platform and their lambda functions
    wolkabout::Timer timerCpuMax;
    // Publish the maximum value within a x given timeframe
    timerCpuMax.run(std::chrono::seconds(CPU_TIMER_MAX),
                    [&]
                    {
                        wolk->addReading("CPU_T_core_max", getMaximumTemperature(temperaturesMax));
                        wolk->publish();
                        LOG(DEBUG) << "Max CPU core temperature is " << getMaximumTemperature(temperaturesMax);
                        std::cout << "Sending max temperature value at time: "
                                  << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl
                                  << "with value: " << getMaximumTemperature(temperaturesMax) << std::endl;
                        temperaturesMax.clear();
                    });
    // Timer for publishing in intervals to the platform and their lambda functions
    wolkabout::Timer timerCpu;
    timerCpu.run(std::chrono::seconds(CPU_TIMER),
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
                     temperatures.clear();
                 });
    // Timer for publishing in intervals to the platform and their lambda functions
    wolkabout::Timer timerIp;
    timerIp.run(std::chrono::seconds(IP_TIMER),
                [&]
                {
                    wolk->addReading("IP_ADD", ip);
                    LOG(DEBUG) << "Ip address published" << ip;
                });
    // LogLevel reading
    wolk->addReading("LOG_LEVEL", deviceInfo.logInfo);
    // And now we will periodically (and endlessly) send a random temperature value.
    while (running)
    {
        std::string newIp = IPAddressReader::retrieveIPAddress();
        temperatures = CpuTemperatureReader::readTemperatures();
        // Add to the temperaturesMax vector only the maximum reading of the core
        temperaturesMax.push_back(getMaximumTemperature(temperatures));

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
