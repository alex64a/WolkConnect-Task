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

#include "core/utilities/Logger.h"
#include "wolk/WolkBuilder.h"
#include "wolk/WolkSingle.h"
#include "cpuTemperatureReader/cpuTemperatureReader.hpp"
#include "ipAddressReader/ipAddressReader.hpp"
#include <random>

/**
 * This is the place where user input is required for running the example.
 * In here, you can enter the device credentials to successfully identify the device on the platform.
 * And also, the target platform path.
 */
const std::string DEVICE_KEY = "AM";
const std::string DEVICE_PASSWORD = "T6RCB4VIE7";
const std::string PLATFORM_HOST = "integration5.wolkabout.com:1883";

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

int main(int /* argc */, char** /* argv */)
{
    //object of class CpuTemperatureReader for reading the core temperatures of the CPU
    //CpuTemperatureReader temperatureReader;

    //object of class IPAddressReader for reading the IP address
    IPAddressReader ipReader;
    // This is the logger setup. Here you can set up the level of logging you would like enabled.
    wolkabout::Logger::init(wolkabout::LogLevel::INFO, wolkabout::Logger::Type::CONSOLE);

    // Here we create the device that we are presenting as on the platform.
    auto device = wolkabout::Device(DEVICE_KEY, DEVICE_PASSWORD, wolkabout::OutboundDataMode::PUSH);

    // And here we create the wolk session
    auto wolk = wolkabout::connect::WolkSingle::newBuilder(device).host(PLATFORM_HOST).buildWolkSingle();
    wolk->connect();



    // And now we will periodically (and endlessly) send a random temperature value.
    while (true)
    {   
        //Read the CPU temperature values
        // std::vector<double> temperatures = temperatureReader.readTemperatures();
        // wolk->addReading("CPU_T_core1", temperatures[0]);
        // wolk->addReading("CPU_T_core2", temperatures[1]);
        // wolk->addReading("CPU_T_core3", temperatures[2]);
        // wolk->addReading("CPU_T_core4", temperatures[3]);

        //Read the IP address
        wolk->addReading("IP_ADD", ipReader.getIPAddress());
        std::this_thread::sleep_for(std::chrono::minutes(1));
        wolk->publish();
    }
    return 0;
}
