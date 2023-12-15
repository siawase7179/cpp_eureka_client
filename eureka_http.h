#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <vector>
#include <map>

#include <mutex>
#include <thread>
#include <exception>

#include <curl/curl.h>

#include "eureka_exception.h"
#include "eureka_response.h"
#include "eureka_json.h"

#ifndef __EUREKA_HTTP_H__
#define __EUREKA_HTTP_H__


namespace eureka{
    using ResponseHeaders = std::map<std::string, std::string>;

    namespace http{
        enum HttpMethod{
            POST = 0,
            GET,
            PUT,
            DELETE
        };

        static ResponseHeaders headers;

        struct Response{
            long httpStatus;
            ResponseHeaders headers;
            std::string response;
        };

        

        class HttpHandler{
        public:
            HttpHandler(int _m_iTimeout = 5, const std::string _userpwd ="") :m_iTimeout(_m_iTimeout), m_userpwd(_userpwd){
                
            }

            Response requset(HttpMethod method, std::string url, std::map<std::string, std::string> headers={}, std::string body="");

        private:

            std::string HttpMethodToString(HttpMethod method) {
                switch (method) {
                    case HttpMethod::POST : return "POST";
                    case HttpMethod::GET: return "GET";
                    case HttpMethod::PUT: return "PUT";
                    case HttpMethod::DELETE: return "DELETE";
                    default: return "Unknown";
                }
            }

            
            int m_iTimeout;
            std::string m_userpwd;
        };
    }

    
}
    
#endif