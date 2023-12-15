#include <thread>
#include <memory>
#include "eureka_client.h"


namespace eureka{

    namespace application{

        res::InstanceInfoPtr Eureka::getInstance(){
            const std::string appName = m_ptr->begin()->first;
            const res::ApplicationPtr appPtr = m_ptr->at(appName);
            const std::vector<res::InstanceInfoPtr>& instances = appPtr->Instances;

            size_t instancesCount = 0;

            for (auto instance : instances){
                if (instance->status == "UP"){
                    instancesCount++;
                }
            }
            if (instancesCount == 0) {
                THROW_EUREKA_EXCEPTION("No instances available:" + appName);
            }

            const res::InstanceInfoPtr instance = instances[m_iCurrentIdx];

            m_iCurrentIdx = (m_iCurrentIdx + 1) % instancesCount;

            return instance;
        }
        
        http::Response Eureka::request(http::HttpHandler httphandler, const http::HttpMethod method, const std::string path, const std::map<std::string, std::string> headers, const std::string body){
            const std::string appName = m_ptr->begin()->first;
            const res::ApplicationPtr appPtr = m_ptr->at(appName);
            const std::vector<res::InstanceInfoPtr>& instances = appPtr->Instances;
            http::Response response;
            do{
                size_t instancesCount = instances.size();
                if (instancesCount == 0) {
                    THROW_EUREKA_EXCEPTION("No instances available:" + appName);
                }

                const res::InstanceInfoPtr instance = getInstance();

                std::string port = std::to_string(instance->port->port);
                std::string url = instance->hostName + ":" + port + path;
                std::cout<<url<<std::endl;
                try{
                    response = httphandler.requset(method, url, headers, body);
                    break;
                }catch(const except::HttpConnectException &e){
                    instance->status = "DOWN";
                }
            }while(true);
            
            return response;
        }
    }
    
    
    http::Response Client::requestEurekaApp(const std::string status){
        rapidjson::Document document;
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();


        rapidjson::Value port(rapidjson::kObjectType);
        port.AddMember("@enabled", false, allocator);

        rapidjson::Value dataCenterInfo(rapidjson::kObjectType);
        dataCenterInfo.AddMember("@class", "com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo", allocator);
        dataCenterInfo.AddMember("name", "MyOwn", allocator);

        char hostname[128];
        memset(hostname, 0x00, sizeof(hostname));
        gethostname(hostname, sizeof(hostname));

        struct hostent * record = gethostbyname(hostname);
        in_addr * address = (in_addr * )record->h_addr;
        std::string ip_address = inet_ntoa(* address);

        rapidjson::Value instance(rapidjson::kObjectType);
        instance.AddMember("port", port, allocator);
        instance.AddMember("dataCenterInfo", dataCenterInfo, allocator);

        instance.AddMember("hostName", ip_address, allocator);
        instance.AddMember("app", appid, allocator);
        instance.AddMember("instanceId", instanceId, allocator);
        instance.AddMember("ipAddr", ip_address, allocator);
        instance.AddMember("status", status, allocator);
        instance.AddMember("app", appid, allocator);

        document.SetObject();
        document.AddMember("instance", instance, allocator);
        

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        std::string jsonString = buffer.GetString();
        std::cout << jsonString << std::endl;
    
        int errorCount = 0;
        int errorEurekaCount=0;
        http::Response response;
        do {
            try {
                response = m_Httphandler.requset(http::POST, m_endpoint+"/eureka/apps/"+appid, {} ,jsonString);
                break;
            } catch (const std::exception &e) {
                std::cerr<<e.what()<<std::endl;
                errorCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (errorCount >= m_iRetryCount) {
                    errorCount = 0;
                    m_endpointsIndex = (m_endpointsIndex + 1) % m_endpoints.size();
                    switchEndpoint(m_endpointsIndex);

                    errorEurekaCount++;
                    if (errorEurekaCount == (int)m_endpoints.size()){
                        THROW_EUREKA_EXCEPTION("Eureka not connected.");
                    }
                    
                }
            }
        } while (true);

        return response;
    }

    http::Response Client::heartbeat(){
        int errorCount = 0;
        int errorEurekaCount = 0;
        http::Response response;
        do{
            try {
                
                response = m_Httphandler.requset(http::PUT, m_endpoint+"/eureka/apps/"+appid+"/"+instanceId);
                break;
            } catch (const std::exception &e) {
                std::cerr<<e.what()<<std::endl;
                errorCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (errorCount >= m_iRetryCount) {
                    errorCount = 0;
                    m_endpointsIndex = (m_endpointsIndex + 1) % m_endpoints.size();
                    switchEndpoint(m_endpointsIndex);
                    
                    errorEurekaCount++;
                    if (errorEurekaCount == (int)m_endpoints.size()){
                        THROW_EUREKA_EXCEPTION("Eureka not connected.");
                    }
                }
            }
        } while (true);

        return response;
    }

    http::Response Client::deleteEurekaeApp(){
        int errorCount = 0;
        int errorEurekaCount = 0;
        http::Response response;
        do {
            try {

                response = m_Httphandler.requset(http::DELETE, m_endpoint+"/eureka/apps/"+appid+"/"+instanceId);
                break;
            } catch (const std::exception &e) {
                std::cerr<<e.what()<<std::endl;
                errorCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (errorCount >= m_iRetryCount) {
                    errorCount = 0;
                    m_endpointsIndex = (m_endpointsIndex + 1) % m_endpoints.size();
                    switchEndpoint(m_endpointsIndex);
                    
                    errorEurekaCount++;
                    if (errorEurekaCount == (int)m_endpoints.size()){
                        THROW_EUREKA_EXCEPTION("Eureka not connected.");
                    }
                }
            }
        } while (true);
        return response;
    }

    http::Response Client::requestInstance(const std::string appid, const http::HttpMethod method, const std::string path, const std::string body){
        http::Response response;

        eureka_lock_mutex.lock();

        if (eurekaApplications.find(appid) == eurekaApplications.end() || time(NULL) - m_lLastLoadTime >= m_iReLoadTime){
            queryInAppId(appid);
            m_lLastLoadTime = time(NULL);
        }else{
            
        }
        eureka_lock_mutex.unlock();

        auto it = eurekaApplications.find(appid);
        if (it != eurekaApplications.end()){
            std::shared_ptr<application::Eureka> eureka = it->second;
            try{
                response = eureka->request(m_Httphandler, method, path, m_headers, body);
            }catch(const except::EurekaException &e){
                THROW_EUREKA_EXCEPTION(e.what());
            }
        }else{
            THROW_EUREKA_EXCEPTION("Unkown Application:" + appid);
        }
        
        return response;
    }



    void Client::queryInAppId(const std::string appid){

        int errorCount = 0;
        int errorEurekaCount = 0;
        ResponseApplication eurekaApp;
        http::Response response;

        do {
            try {
                
                response = m_Httphandler.requset(eureka::http::HttpMethod::GET, m_endpoint + "/eureka/apps/" + appid);
                std::cout<<response.response<<std::endl;
                break;
            } catch (const std::exception &e) {
                std::cerr<<e.what()<<std::endl;
                errorCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (errorCount >= m_iRetryCount) {
                    errorCount = 0;
                    m_endpointsIndex = (m_endpointsIndex + 1) % m_endpoints.size();
                    switchEndpoint(m_endpointsIndex);

                    errorEurekaCount++;
                    if (errorEurekaCount == (int)m_endpoints.size()){
                        THROW_EUREKA_EXCEPTION("Eureka not connected.");
                    }
                }
            }
        } while (true);

        if (response.httpStatus == 200){
            json::JsonReader reader(response.response);
            res::ApplicationPtr application = reader.readIntance();

            eurekaApp[appid] = application;
            
            
            application::Eureka eureka(appid, std::make_shared<ResponseApplication>(eurekaApp));
            if (eurekaApplications.find(appid) != eurekaApplications.end()){
                eurekaApplications.erase(appid);
            }
            eurekaApplications[appid] = std::make_shared<application::Eureka>(eureka);
        }else{

        }

        
    }

    void Client::addHeader(const std::string name, const std::string value){
        if(m_headers.find(name) != m_headers.end()){
            m_headers.erase(name);
            m_headers[name] = value;
        }else{
            m_headers[name] = value;
        }
    }
}
