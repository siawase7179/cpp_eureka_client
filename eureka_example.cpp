#include <thread>
#include <memory>
#include "eureka_client.h"

bool bStopFlag = false;

void threadFunction(std::shared_ptr<eureka::Client> client) {
    while (!bStopFlag) {
        try {
            sleep(2);
            client->heartbeat();
        } catch (const eureka::except::EurekaException& e) {
            std::cout << "error" << e.what() << std::endl;
            exit(0);
        }
    }
}

std::string makeBody(std::string clientId, std::string clientPassword){
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember("clientId", clientId, allocator);
    document.AddMember("clientPassword", clientPassword, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);

    return sb.GetString();
}

int main (int argc, char *argv[]){
    int   param_opt;
    std::string endpoints;
    std::string appid = "AUTH-SERVER";

    while( -1 !=( param_opt = getopt( argc, argv, "h:"))) {
        switch( param_opt) {
            case 'h':
                endpoints = optarg;
            break;
            default: 
                std::cerr<<"Usage:"<<std::endl;
            break;
        }
    }

    if (endpoints.size() <= 0){
        return -1;
    }

    std::shared_ptr<eureka::Client> client = std::make_shared<eureka::Client>("C-SERVICE", endpoints);
    client->requestEurekaApp("UP");
    sleep(1);
    std::thread t1(threadFunction, client);
    while (!bStopFlag){
        try{
            eureka::http::Response response =  client->requestInstance(appid, eureka::http::POST, "/auth", makeBody("id", "pass"));
            if (response.httpStatus == 200){
                std::cout<<"response:"<<response.response<<std::endl<<std::endl;

                rapidjson::Document document;
                document.Parse(response.response);
                // std::cout<<"token:"<<document["token"].GetString()<<std::endl;
                // std::cout<<"expiry:"<<document["expiry"].GetInt()<<std::endl;
            }
            
            sleep(3);
        }catch (const eureka::except::EurekaException& e){
            std::cout<<__func__<<":"<<__LINE__<<">>"<<e.what()<<std::endl;
            bStopFlag = true;
        }
    }
    t1.join();

    sleep(1);
    client->requestEurekaApp("DOWN");
    return 0;
}