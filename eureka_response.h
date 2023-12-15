#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <map>

#ifndef __EUREKA_RESPONSE_H__
#define __EUREKA_RESPONSE_H__

namespace eureka{
    namespace res{
        using Metadata = std::map<std::string, std::string>;
        using MetadataPtr = std::shared_ptr<Metadata>;

        using StringList = std::vector<std::string>;

        enum class CheckStatus
        {
            UP,
            DOWN,
            OUT_OF_SERVICE,
        };

        struct Port
        {
            int port{0};
            bool enable{false};
        };
        using PortPtr = std::shared_ptr<Port>;

        struct LeaseInfo
        {
            int64_t renewalIntervalInSecs{30};
            int64_t durationInSecs{90};
            int64_t registrationTimestamp{0};
            int64_t lastRenewalTimestamp{0};
            int64_t evictionTimestamp{0};
            int64_t serviceUpTimestamp{0};
        };
        using LeaseInfoPtr = std::shared_ptr<LeaseInfo>;

        struct DataCenterInfo
        {
            std::string name;
            std::string className;
        };
        using DataCenterInfoPtr = std::shared_ptr<DataCenterInfo>;

        struct InstanceInfo{
            std::string app;
            std::string instanceId;
            std::string ipAddr;

            CheckStatus statusCheck;
            PortPtr     port;
            PortPtr     securePort;

            std::string hostName;
            std::string homePageUrl;
            std::string statusPageUrl;
            std::string healthCheckUrl;
            
            std::string vipAddress;
            std::string secureVipAddress;
            std::string status;
            
            DataCenterInfoPtr dataCenterInfo;
            LeaseInfoPtr      leaseInfo;
            MetadataPtr       metadata;

            bool        isCoordinatingDiscoveryServer{false};
            int64_t     lastUpdatedTimestamp{0};
            int64_t     lastDirtyTimestamp{0};
            std::string actionType;
            std::string overriddenstatus;
            int64_t     countryId{0};
        };
        using InstanceInfoPtr = std::shared_ptr<InstanceInfo>;

        struct Application{
            std::string name;
            std::vector<InstanceInfoPtr> Instances;
        };
        using ApplicationPtr = std::shared_ptr<Application>;
    };
};

#endif