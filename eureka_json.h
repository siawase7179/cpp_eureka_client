#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>


#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/writer.h"		// for stringify JSON
#include "rapidjson/stringbuffer.h" // wrapper of C stream for prettywriter as output


#include "eureka_response.h"
#include "eureka_exception.h"

#ifndef __EUREKA_JSON_H__
#define __EUREKA_JSON_H__

namespace eureka{
    namespace json{
        class JsonReader{
            private:
                rapidjson::Document document;

            public:
                JsonReader(std::string message){
                    try{
                        document.Parse(message.c_str());
                    }catch(const std::exception& e){
                        throw except::JsonParsingException(e.what());
                    }
                }

                std::string getString(rapidjson::Value& value, std::string key){
                    if (value.HasMember(key)){
                        if (value[key].IsString()){
                            return value[key].GetString();
                        }else{
                           throw except::JsonParsingException(key + " is not String.");
                        }
                    }else{
                        throw except::JsonParsingException(key + " is not member.");
                    }
                }
                
                res::ApplicationPtr readIntance(){
                    res::ApplicationPtr applicationPtr = std::make_shared<res::Application>();
                    applicationPtr->name = (document["application"]["name"]).GetString();
                    
                    for (auto& instance : document["application"]["instance"].GetArray()){
                        res::InstanceInfoPtr instancePtr = std::make_shared<res::InstanceInfo>();
                        ;
                        instancePtr->instanceId = getString(instance, "instanceId");
                        instancePtr->hostName =  getString(instance, "hostName");
                        instancePtr->app = getString(instance, "app");
                        instancePtr->ipAddr = getString(instance, "ipAddr");
                        instancePtr->status = getString(instance, "status");

                        
                        res::PortPtr portPtr = std::make_shared<res::Port>();
                        rapidjson::Value& port = instance["port"];
                        portPtr->port = port["$"].GetInt();
                        if (strcmp(port["@enabled"].GetString(), "true")==0){
                            portPtr->enable = true;
                        }else{
                            portPtr->enable = false;
                        }
                        instancePtr->port = portPtr;

                        res::PortPtr securePortPtr = std::make_shared<res::Port>();
                        rapidjson::Value& sercurePort = instance["securePort"];
                        securePortPtr->port = sercurePort["$"].GetInt();
                        if (strcmp(sercurePort["@enabled"].GetString(), "true")==0){
                            securePortPtr->enable = true;
                        }else{
                            securePortPtr->enable = false;
                        }
                        instancePtr->securePort = securePortPtr;

                        instancePtr->countryId = instance["countryId"].GetInt();

                        res::DataCenterInfoPtr dataCenterInfoPtr = std::make_shared<res::DataCenterInfo>();
                        rapidjson::Value& dataCenterInfo = instance["dataCenterInfo"];
                        dataCenterInfoPtr->className = getString(dataCenterInfo, "@class");
                        dataCenterInfoPtr->name = getString(dataCenterInfo, "name");
                        instancePtr->dataCenterInfo = dataCenterInfoPtr;

                        res::LeaseInfoPtr leaseInfoPtr = std::make_shared<res::LeaseInfo>();
                        rapidjson::Value& leaseInfo = instance["leaseInfo"];
                        leaseInfoPtr->renewalIntervalInSecs = leaseInfo["renewalIntervalInSecs"].GetInt();
                        leaseInfoPtr->durationInSecs = leaseInfo["durationInSecs"].GetInt();
                        leaseInfoPtr->registrationTimestamp = leaseInfo["registrationTimestamp"].GetInt64();
                        leaseInfoPtr->lastRenewalTimestamp = leaseInfo["lastRenewalTimestamp"].GetInt64();
                        leaseInfoPtr->evictionTimestamp = leaseInfo["evictionTimestamp"].GetInt64();
                        leaseInfoPtr->serviceUpTimestamp = leaseInfo["serviceUpTimestamp"].GetInt64();
                        instancePtr->leaseInfo = leaseInfoPtr;

                        res::Metadata metadataMap;
                        rapidjson::Value& metadata = instance["metadata"];
                        for (rapidjson::Value::ConstMemberIterator it = metadata.MemberBegin(); it != metadata.MemberEnd(); ++it) {
                            if (it->value.IsString()) {
                                metadataMap[it->name.GetString()] = it->value.GetString();
                            }
                        }
                        instancePtr->metadata = std::make_shared<res::Metadata>(metadataMap);

                        instancePtr->homePageUrl = getString(instance, "homePageUrl");
                        instancePtr->statusPageUrl = getString(instance, "statusPageUrl");
                        instancePtr->healthCheckUrl = getString(instance, "healthCheckUrl");
                        instancePtr->vipAddress = getString(instance, "vipAddress");
                        instancePtr->secureVipAddress = getString(instance, "secureVipAddress");
                        if (getString(instance, "isCoordinatingDiscoveryServer") == "true"){
                            instancePtr->isCoordinatingDiscoveryServer = true;
                        }
                        instancePtr->lastUpdatedTimestamp = std::stoll(getString(instance, "lastUpdatedTimestamp"));
                        instancePtr->lastDirtyTimestamp = std::stoll(getString(instance, "lastDirtyTimestamp"));
                        instancePtr->actionType = getString(instance, "actionType");

                         if (instancePtr->status == "UP"){
                            applicationPtr->Instances.push_back(instancePtr);
                        } 
                    }

                    return applicationPtr;
                }
        };
    };
}

#endif