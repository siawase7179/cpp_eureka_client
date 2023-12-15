#include "eureka_http.h"

namespace eureka{
    namespace http{
        static size_t AckResPrint(void *ptr, size_t size, size_t count, void *stream) {
            ((std::string*) stream)->append((char*) ptr, 0, size * count);
            return size * count;
        }
        

        static size_t headerCallback(void *contents, size_t size, size_t nmemb, void *userp) {
            size_t totalSize = size * nmemb;
            std::string headerLine(static_cast<char*>(contents), totalSize);

            size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos) {
                std::string key = headerLine.substr(0, colonPos);
                std::string value = headerLine.substr(colonPos + 2, totalSize - colonPos - 3);
                headers[key] = value;
            }

            return totalSize;
        }    

        Response HttpHandler::requset(HttpMethod method, std::string url, std::map<std::string, std::string> headers, std::string body){
            CURLcode res;
            Response response;

            curl_global_init(CURL_GLOBAL_ALL);
            CURL *m_pCurl = curl_easy_init();
            if (m_pCurl == NULL){
                throw except::HttpConnectException("m_pCurl is Null!");
            }
            struct curl_slist *m_Header = NULL;
            

            m_Header = curl_slist_append(m_Header, "Content-Type: application/json");
            m_Header = curl_slist_append(m_Header, "Accept: application/json");
            
            for (auto iter : headers){
                std::string header = iter.first + ":" + iter.second;
                m_Header = curl_slist_append(m_Header, header.c_str());
            }

            curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_Header);
            curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, eureka::http::headerCallback);
            curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, eureka::http::AckResPrint);
            curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response.response);
            curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, m_iTimeout * 1000);

            if (m_userpwd.size() > 0){
                curl_easy_setopt(m_pCurl, CURLOPT_USERPWD, m_userpwd.c_str());
            }

            if (method == HttpMethod::POST){
                curl_easy_setopt(m_pCurl, CURLOPT_POST, true);
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, body.c_str());
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, body.size());
            }else if (method == HttpMethod::GET){
                curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "GET");
            }else if (method == HttpMethod::PUT){
                curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "PUT");
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, body.c_str());
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, body.size());
            }else{
                curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
            }
            
            res = curl_easy_perform(m_pCurl);
            if (res == CURLE_OK) {
                curl_easy_getinfo(m_pCurl, CURLINFO_HTTP_CODE, &response.httpStatus);
                response.headers = headers;                  
            }else{
                if (m_Header != NULL){
                    curl_slist_free_all(m_Header);
                    m_Header = NULL;
                }
                if (m_pCurl != NULL ) {
                    curl_easy_cleanup(m_pCurl);
                    curl_global_cleanup();
                    m_pCurl = NULL;
                }
                throw except::HttpConnectException((std::string)curl_easy_strerror(res) + ":" + url);
            }
            if (m_Header != NULL){
                curl_slist_free_all(m_Header);
                m_Header = NULL;
            }
            if (m_pCurl != NULL ) {
                curl_easy_cleanup(m_pCurl);
                curl_global_cleanup();
                m_pCurl = NULL;
            }
            return response;

        }
    }
}
