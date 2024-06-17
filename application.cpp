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
  wolkabout::LogLevel logInfo;
  
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
                // std::lock_guard<std::mutex> lock{mutex};
             
                

                
            }

            // Notify the condition variable
            conditionVariable.notify_one();
        }
    }

private:
    // This is where the object containing all information about the device is stored.
    DeviceData& m_deviceData;
};

/**
 * This is an example implementation of the `FileListener` interface. This class will log when a file gets
 * added/removed.
 */
class applicationFileListener : public wolkabout::connect::FileListener
{
public:
    /**
     * This is an overridden method from the `FileListener` interface. This is a method that will be invoked once a file
     * has been added.
     *
     * @param deviceKey The device key for the file that has been added.
     * @param fileName The name of the file that has been added.
     * @param absolutePath The absolute path to the file that has been added.
     */
    void onAddedFile(const std::string& deviceKey, const std::string& fileName,
                     const std::string& absolutePath) override
    {
        LOG(INFO) << "A file has been added! -> '" << fileName << "' | '" << absolutePath << "' (on device '"
                  << deviceKey << "').";
    }

    /**
     * This is an overridden method from the `FileListener` interface. This is a method that will be invoked once a file
     * has been removed.
     *
     * @param deviceKey The device key for the file that has been removed.
     * @param fileName The name of the file that has been removed.
     */
    void onRemovedFile(const std::string& deviceKey, const std::string& fileName) override
    {
        LOG(INFO) << "A file has been removed! -> '" << fileName << "' (on device '" << deviceKey << "').";
    }
};



/**
 * This is a function that will generate a random Temperature value for us.
 *
 * @return A new Temperature value, in the range of -20 to 80.
 */
std::uint64_t generateRandomValue()
{
    // Here we will create the random engine and distribution
    static auto engine =
      std::mt19937(static_cast<std::uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    static auto distribution = std::uniform_real_distribution<>(-20, 80);

    // And generate a random value
    return static_cast<std::uint64_t>(distribution(engine));
}


//A function to return the maximum value of all CPU core temperatures
double publishMaximumTemperature(std::vector<double>&temperatures) {
    

    double maxValue = temperatures[0];
    for(auto itr : temperatures)
    {
        if (temperatures[itr] > maxValue)
        maxValue = temperatures[itr];
    }
    return maxValue;
}


/**
 * This is interrupt logic used to stop the application from running.
 */
std::function<void(int)> sigintCall;

void sigintResponse(int signal)
{
    if (sigintCall != nullptr)
        sigintCall(signal);
}


int main(int /* argc */, char** /* argv */)
{
    //object of class CpuTemperatureReader for reading the core temperatures of the CPU
    CpuTemperatureReader temperatureReader;

    //object of class IPAddressReader for reading the IP address
    IPAddressReader ipReader;
    // This is the logger setup. Here you can set up the level of logging you would like enabled.
    wolkabout::Logger::init(wolkabout::LogLevel::INFO, wolkabout::Logger::Type::CONSOLE);
    //wolkabout::Logger::init(wolkabout::LogLevel::DEBUG, wolkabout::Logger::Type::FILE, "/home/amitrovcan/LogInfo/log*.txt");
    // wolkabout::Logger::setupConsoleLogger();
    //wolkabout::Logger::setupFileLogger("/home/amitrovcan/LogInfo/log*.txt");

    // Here we create the device that we are presenting as on the platform.
    auto device = wolkabout::Device(DEVICE_KEY, DEVICE_PASSWORD, wolkabout::OutboundDataMode::PUSH);
    auto deviceInfo = DeviceData{{0.0}, "", wolkabout::LogLevel::INFO};
    auto deviceInfoHandler = std::make_shared<DeviceDataChangeHandler>(deviceInfo);


    // And here we create the wolk session
    auto wolk = wolkabout::connect::WolkBuilder(device)
                  .host(PLATFORM_HOST)
                  .feedUpdateHandler(deviceInfoHandler)
                  .withFileListener(std::make_shared<applicationFileListener>())
                  .buildWolkSingle();


    //auto wolk = wolkabout::connect::WolkSingle::newBuilder(device).host(PLATFORM_HOST).buildWolkSingle();
    wolk->connect();
    bool running = true;
    sigintCall = [&](int) {
        LOG(WARN) << "Application: Received stop signal, disconnecting...";
        conditionVariable.notify_one();
        running = false;
    };

    wolk->obtainDetails([&](const std::vector<std::string>& feeds, const std::vector<std::string>& attributes) {
        LOG(INFO) << "Received device details: ";
        LOG(INFO) << "\tFeeds: ";
        for (const auto& feed : feeds)
            LOG(INFO) << "\t\t" << feed;
        LOG(INFO) << "\tAttributes: ";
        for (const auto& attribute : attributes)
            LOG(INFO) << "\t\t" << attribute;
    });

        //Read the CPU temperature values and the IP address (initial value)
        std::vector<double> temperatures = temperatureReader.readTemperatures();
        std::string ip = ipReader.getIPAddress();
        deviceInfo.temperatures = temperatures;
        deviceInfo.ipAddress = ip;

    // And now we will periodically (and endlessly) send a random temperature value.
    while (running)
    {   
        std::string newIp = ipReader.getIPAddress();
   
    
        wolk->addReading("CPU_T_core1", deviceInfo.temperatures[0]);
        wolk->addReading("CPU_T_core2", deviceInfo.temperatures[1]);
        wolk->addReading("CPU_T_core3", deviceInfo.temperatures[2]);
        wolk->addReading("CPU_T_core4", deviceInfo.temperatures[3]);
        // std::this_thread::sleep_for(std::chrono::minutes(1));
        wolk->addReading("IP_ADD", deviceInfo.ipAddress);
        wolk->addReading("CPU_T_core_max", publishMaximumTemperature(temperatures));
      //  std::this_thread::sleep_for(std::chrono::minutes(5));


        //Publish new IP_ADD only if it has changed 
        if(deviceInfo.ipAddress != newIp){
        wolk->addReading("IP_ADD", ip);
        wolk->publish();
        ip = newIp;
        deviceInfo.ipAddress = ip;
        LOG(INFO) << "\t IP_ADD changed";
        LOG(INFO) << "NEW IP: " << ip;
        std::this_thread::sleep_for(std::chrono::minutes(5));
        }

        //LogLevel
        wolk->addReading("LOG_LEVEL", wolkabout::LogLevel::INFO);
        wolk->addReading("LOG_LEVEL", wolkabout::LogLevel::DEBUG);
        //Temperature that is randomly generated
        wolk->addReading("T", generateRandomValue());
        std::this_thread::sleep_for(std::chrono::minutes(1));
        wolk->publish();
    
    }
    return 0;
}
