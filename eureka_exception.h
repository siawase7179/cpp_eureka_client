#include <string>
#include <exception>

#ifndef __EUREKA_EXCEPTION_H__
#define __EUREKA_EXCEPTION_H__

#ifndef THROW_EUREKA_EXCEPTION
#define THROW_EUREKA_EXCEPTION(message) \
    throw eureka::except::EurekaException(message, basename(__FILE__), __LINE__)
#endif


namespace eureka{
    namespace except{
        class HttpConnectException :public std::exception {
            public:
                HttpConnectException(const std::string& _message) : message(_message){

                }
                virtual const char* what() const throw() {
                    return message.c_str();
                }
            private:
                std::string message;
        };

        class JsonParsingException :public std::exception{
            public:
                JsonParsingException(const std::string& _message) : message(_message){

                }
                virtual const char* what() const throw() {
                    return message.c_str();
                }
            private:
                std::string message;
        };

        class EurekaException :public std::exception {
            public:
                EurekaException(const std::string& _message, const char* _file, const int _line) : message(_message), file(_file), line(_line){

                }
                virtual const char* what() const throw() {
                    static std::string errorMessage("(" + (std::string)file + ":" + std::to_string(line) + ") " + message);
                    return errorMessage.c_str();
                }
            private:
                std::string message;
                const char * file;
                const int line;
        };
    }
}

#endif