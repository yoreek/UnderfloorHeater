#pragma once
#ifndef _MY_HEATER_UI_H_
#define _MY_HEATER_UI_H_

#include <SdFat.h>
#include <WebServer.h>

#define MY_HEATER_UI_BUFFER_LEN    256
#define MY_HEATER_UI_MIN_SOCK_NUM  1
#define MY_HEATER_UI_MAX_SOCK_NUM  4

class MyHeaterUI: public WebServer {
    public:
        static char requestBuffer[MY_HEATER_UI_BUFFER_LEN];
        SdFat      *sd;
        File        fh;
        const char *authCredentials;

        typedef void MyHeaterUICommand(
            MyHeaterUI &server, ConnectionType type,
            char *url_tail, bool tail_complete);

        typedef void MyHeaterUIUrlPathCommand(
            MyHeaterUI &server, ConnectionType type,
            char **url_path, char *url_tail,
            bool tail_complete);

        MyHeaterUI(const char *prefix, int port,
            const char *authCredentials, SdFat *sd);

        void _setDefaultCommand(MyHeaterUICommand *cmd);
        void _setFailureCommand(MyHeaterUICommand *cmd);
        void _addCommand(const char *verb, MyHeaterUICommand *cmd);
        void _setUrlPathCommand(MyHeaterUIUrlPathCommand *cmd);

        void maintain();
};

#endif
