#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <tuple>
#include <curl/curl.h>
#include <vector>
#include <sstream>
#include <stdexcept>

#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <assert.h>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <netdb.h>
#include <arpa/inet.h>


#include "eureka_response.h"
#include "eureka_json.h"
#include "eureka_http.h"
#include "eureka_exception.h"

#ifndef __EUREKA_CLIENT_H__
#define __EUREKA_CLIENT_H__

namespace eureka{
    using StringList = std::vector<std::string>;
    using ResponseApplication = std::map<std::string, res::ApplicationPtr>;
    using ResponseApplicationPtr = std::shared_ptr<ResponseApplication>;

    namespace application {
        class Eureka{
            public:
                Eureka(std::string _appid, ResponseApplicationPtr _ptr)
                    : m_iCurrentIdx(0), m_strAppid(_appid), m_ptr(_ptr){

                }

                res::InstanceInfoPtr getInstance();

                http::Response request(http::HttpHandler httphandler, const http::HttpMethod method, const std::string path, std::map<std::string, std::string> headers={}, const std::string body="");

            private:
                int m_iCurrentIdx;
                std::string m_strAppid;
                ResponseApplicationPtr m_ptr;
        };
    }

    class Client{
        public:
            Client(const std::string _appid, const std::string endpoints, const std::string userpwd="", const int iTimeout = 3, const int _m_iRetryCount = 3)
                :m_pCurl(NULL), m_Httphandler(iTimeout, userpwd), m_endpointsIndex(0), appid(_appid),
                m_iTimeout(iTimeout), m_iRetryCount(_m_iRetryCount),
                m_lLastLoadTime(0), m_iReLoadTime(30) {
                
                std::istringstream ss(endpoints);
                std::string endpoint;

                while (std::getline(ss, endpoint, ',')) {
                    m_endpoints.push_back(endpoint);
                }
                char hostname[128];
                memset(hostname, 0x00, sizeof(hostname));
                gethostname(hostname, sizeof(hostname));

                instanceId = (std::string)hostname + ":" + appid;

                switchEndpoint(m_endpointsIndex);
            }
            
            ~Client(){
                if (NULL != m_pCurl) {
                    curl_easy_cleanup(m_pCurl);
                    curl_global_cleanup();
                }
                m_pCurl = NULL;
            }

            http::Response requestEurekaApp(const std::string status);

            http::Response heartbeat();

            http::Response deleteEurekaeApp();

            http::Response requestInstance(const std::string appid, const http::HttpMethod method, const std::string path, const std::string body="");

            void queryInAppId(const std::string appid);
            void addHeader(const std::string name, const std::string value);
            
        private:
            void setEndpoint(const std::string &endpoint){            
                m_endpoint = endpoint;
            }
    
            void switchEndpoint(std::size_t endpointIndex){
                if (m_endpoints.empty())
                {
                    m_endpointsIndex = 0;
                    return;
                }
                m_endpointsIndex = endpointIndex % m_endpoints.size();

                setEndpoint(currentEndPoint());
            }

            std::string currentEndPoint() const{
                if (m_endpoints.empty())
                    return "";
                return m_endpoints[m_endpointsIndex % m_endpoints.size()];
            }

            CURL *m_pCurl;

            StringList m_endpoints;
            std::string m_endpoint;
            
            std::string appid;
            std::string instanceId;
            
            int m_iRetryCount;

            int m_endpointsIndex;
            int m_iTimeout;
            std::map<std::string, std::shared_ptr<application::Eureka>> eurekaApplications;

            http::HttpHandler m_Httphandler;

            std::mutex eureka_lock_mutex;

            std::map<std::string, std::string> m_headers;
            long m_lLastLoadTime;
            int m_iReLoadTime;
    };
}

#endif