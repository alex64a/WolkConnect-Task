/*
 * Copyright 2017 WolkAbout Technology s.r.o.
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

#ifndef MQTTCONNECTIVITYSERVICE_H
#define MQTTCONNECTIVITYSERVICE_H

#include "connectivity/ConnectivityService.h"
#include "connectivity/mqtt/MqttClient.h"
#include "connectivity/mqtt/PahoMqttClient.h"
#include "model/Device.h"
#include "model/Reading.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace wolkabout
{
class MqttConnectivityService : public ConnectivityService
{
public:
    MqttConnectivityService(std::shared_ptr<MqttClient> mqttClient, Device device, std::string host);
    virtual ~MqttConnectivityService() = default;

    bool connect() override;
    void disconnect() override;

    bool isConnected() override;

    bool publish(std::shared_ptr<OutboundMessage> outboundMessage) override;

private:
    std::shared_ptr<MqttClient> m_mqttClient;
    Device m_device;
    std::string m_host;

    std::vector<std::string> m_subscriptionList;

    std::atomic_bool m_connected;

    static const constexpr char* LAST_WILL_TOPIC_ROOT = "lastwill/";
    static const constexpr char* ACTUATION_REQUEST_TOPIC_ROOT = "actuators/commands/";

    static const constexpr char* TRUST_STORE = "ca.crt";
};
}

#endif