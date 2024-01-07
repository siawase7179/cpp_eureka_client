# c/c++ Eureka 통신

c/c++ 에서는 eureka 라이브러리 및 패키지가 없기 때문에 구현해 보았다.

```cmake
find_package(RapidJSON)

link_libraries(
    curl
)
```
json을 읽기 위해서 RapidJSON을 사용하였고, 경량화를 위해 curl을 사용하였다.

```c++
std::shared_ptr<eureka::Client> client = std::make_shared<eureka::Client>("C-SERVICE", endpoints);
client->requestEurekaApp("UP");
```
eureka::Client 클래서 생성 후 requestEurekaApp 함수로 UP을 호출하면 Eureka서버에 등로된다.

```c++
try{
    eureka::http::Response response =  client->requestInstance(appid, eureka::http::POST, "/auth", makeBody("id", "pass"));
    if (response.httpStatus == 200){
        std::cout<<"response:"<<response.response<<std::endl<<std::endl;

        rapidjson::Document document;
        document.Parse(response.response);
        std::cout<<"token:"<<document["token"].GetString()<<std::endl;
        std::cout<<"expiry:"<<document["expiry"].GetInt()<<std::endl;
    }
    
}catch (const eureka::except::EurekaException& e){
    std::cout<<__func__<<":"<<__LINE__<<">>"<<e.what()<<std::endl;
}
```
appid에 통신할 Eureka Client Application 이름을 넣어주고 경로, request body를 넣어주면 라운드 로빈으로 Eureka Client Application간 통신이 가능하다.

Instance 하나가 연결이 안될 시 자동으로 라운드로빈 타겟에서 제거되고

30초(default) 후 Eureka 서버로 부터 해당 Application 정보를 재로딩 하여 다시 라운드 로빈이 된다.

