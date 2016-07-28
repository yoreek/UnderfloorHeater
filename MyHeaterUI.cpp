#include <errno.h>
#include <SdFat.h>
#include <Time.h>
#include <Heater/ScheduledHeater.h>
#include <Sensor/Temperature/TemperatureSensor.h>
#include <DebugUtil.h>
#include <StringUtil.h>
#include "MyHeater.h"
#include "MyHeaterUI.h"
#include "MyRemoteSensors.h"

#define MY_HEATER_UI_NAME_LEN   32
#define MY_HEATER_UI_VALUE_LEN  256
#define MY_HEATER_UI_MODE_MIN   0
#define MY_HEATER_UI_MODE_MAX   3
#define MY_HEATER_UI_TEMP_MIN   0.0
#define MY_HEATER_UI_TEMP_MAX   30.0
#define MY_HEATER_UI_PERIOD_MIN 0
#define MY_HEATER_UI_PERIOD_MAX (24 * 60)

using namespace DebugUtil;

extern MyHeater *myHeater;

char MyHeaterUI::requestBuffer[MY_HEATER_UI_BUFFER_LEN];

static bool getNextStrParam(MyHeaterUI &server, char **url_tail, const char *param, char *value) {
    URLPARAM_RESULT rc;
    char name[MY_HEATER_UI_NAME_LEN];

    while ((*url_tail)[0] != '\0') {
        rc = server.nextURLparam(url_tail, name, MY_HEATER_UI_NAME_LEN, value, MY_HEATER_UI_VALUE_LEN);
        if (rc != URLPARAM_EOS) {
            if (!strcmp(name, param)) {
                return true;
            }
        }
    }

    return false;
}

/*
static bool getIntParam(MyHeaterUI &server, char *url_tail, const char *param, long int *result) {
    char *endptr;
    URLPARAM_RESULT rc;
    char name[MY_HEATER_UI_NAME_LEN];
    char value[MY_HEATER_UI_VALUE_LEN];

    if (strlen(url_tail)) {
        while ( strlen(url_tail) ) {
            rc = server.nextURLparam(&url_tail, name, MY_HEATER_UI_NAME_LEN, value, MY_HEATER_UI_VALUE_LEN);
            if (rc != URLPARAM_EOS) {
                if (!strcmp(name, param)) {
                    *result = strtol(value, &endptr, 10);
                    if (errno || endptr == value) return false;
                    return true;
                }
            }
        }
    }

    return false;
}

static bool getFloatParam(MyHeaterUI &server, char *url_tail, const char *param, float *result) {
    char *endptr;
    URLPARAM_RESULT rc;
    char name[MY_HEATER_UI_NAME_LEN];
    char value[MY_HEATER_UI_VALUE_LEN];

    if (strlen(url_tail)) {
        while ( strlen(url_tail) ) {
            rc = server.nextURLparam(&url_tail, name, MY_HEATER_UI_NAME_LEN, value, MY_HEATER_UI_VALUE_LEN);
            if (rc != URLPARAM_EOS) {
                if (!strcmp(name, param)) {
                    *result = strtod(value, &endptr);
                    if (errno || endptr == value) return false;
                    return true;
                }
            }
        }
    }

    return false;
}
*/

static void webApiHeatersGet(MyHeaterUI &server, WebServer::ConnectionType type, char *, bool) {
    int                      i, day, period;
    ScheduledHeater         *heater;
    TemperatureSensorCached *sensor;
    MaintainedObject        *obj;

    if (server.authCredentials != NULL && !server.checkCredentials(server.authCredentials)) {
        server.httpUnauthorized();
        return;
    }

    server.httpSuccess("application/json; charset=utf-8");

    server.print("{\"heaters\":[");

    obj = myHeater->heaters()->first();
    i = 0;
    while (obj != NULL) {
        heater = (ScheduledHeater *) obj;
        sensor = (TemperatureSensorCached *) heater->getTemperatureSensor();

        if (i > 0) {
            server.print(",");
        }

        server.print("{");

        server.print("\"name\":\"");
        server.print(heater->name());
        server.print("\"");
        server.print(",\"mode\":");
        server.print(heater->getMode());
        server.print(",\"targetTemperature\":");
        server.print(heater->getTargetTemperature());
        server.print(",\"hysteresis\":");
        server.print(heater->getHysteresis());
        server.print(",\"awayTemperature\":");
        server.print(heater->awayTemperature());
        server.print(",\"regulated\":");
        server.print(heater->isRegulated() ? "true" : "false");
        server.print(",\"turned\":");
        server.print(heater->isTurnedOn() ? "true" : "false");

        server.print(",\"schedule\":[");
        for (day = 0; day < 7; day++) {
            if (day > 0) server.print(",");
            server.print("[");
            for (period = 0; period < 5; period++) {
                if (period > 0) server.print(",");
                server.print("{");
                server.print("\"start\":");
                server.print(heater->schedule[day][period].start);
                server.print(",\"end\":");
                server.print(heater->schedule[day][period].end);
                server.print(",\"setpoint\":");
                server.print(heater->schedule[day][period].setpoint);
                server.print("}");
            }
            server.print("]");
        }
        server.print("]");


        server.print(",\"sensor\": {");
        {
            float temp = sensor->update()
                ? sensor->getTemperature()
                : 999;
            server.print("\"temperature\":");
            server.print(temp);
        }
        server.print("}}");

        obj = obj->next;
        i++;
    }

    server.print("],\"remoteSensors\":[");

    obj = myHeater->remoteSensors()->first();
    i = 0;
    while (obj != NULL) {
        if (i > 0) {
            server.print(",");
        }

        ((MyRemoteSensor *) obj)->printJson(server);

        obj = obj->next;
        i++;
    }

    server.print("],\"system\":{");
    {
        server.print("\"ramRree\":");
        server.print(ramFree());
        server.print(",\"ramSize\":");
        server.print(ramSize());
    }
    server.print("}");

    server.print("}");
}

static void webFileAccess(MyHeaterUI &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete) {
    char *p, *ext;

    if (server.authCredentials != NULL && !server.checkCredentials(server.authCredentials)) {
        server.httpUnauthorized();
        return;
    }

    if (!tail_complete || *url_path == NULL) {
        server.httpServerError();
        return;
    }

    // root
    DEBUG("go to root");
    if (!server.sd->chdir()) {
        DEBUG("ERROR - SD card failed!");
        server.httpServerError();
        server.print("<h1>SD card failed</h1>");
        return;
    }

    while ((p = *url_path++) != NULL) {
        if (*url_path == NULL) {
            break;
        }
        DEBUG("chdir: %s", p);
        if (!server.sd->chdir(p)) {
            DEBUG("ERROR - Can't chdir!");
            server.httpNotFound();
            return;
        }
    }

    DEBUG("check if exists: %s", p);
    if (!server.sd->exists(p)) {
        DEBUG("ERROR - File not found! %s", p);
        server.httpNotFound();
        return;
    }

    DEBUG("open file: %s", p);
    server.fh = server.sd->open(p);
    if (!server.fh) {
        DEBUG("ERROR - Can't open file!");
        server.httpServerError();
        return;
    }
    dir_t dir;
    if (!server.fh.dirEntry(&dir)) {
        DEBUG("dirEntry failed");
        server.httpServerError();
        return;
    }

    ext = strrchr(p, '.');
    if (ext == NULL) {
        server.httpSuccess();
    }
    else {
        ext++;
        // Last-Modified: Tue, 15 Nov 1994 12:45:26 GMT
        char header[128];
        char day[4];
        char month[4];
        TimeElements tm;
        uint16_t fd = dir.lastWriteDate;
        uint16_t ft = dir.lastWriteTime;
        tm.Day     = FAT_DAY(fd);
        tm.Month   = FAT_MONTH(fd);
        tm.Year    = CalendarYrToTm(FAT_YEAR(fd));
        tm.Hour    = FAT_HOUR(ft);
        tm.Minute  = FAT_MINUTE(ft);
        tm.Second  = FAT_SECOND(ft);
        breakTime(makeTime(tm), tm);
        strncpy(day, dayShortStr(tm.Wday), sizeof(day));
        strncpy(month, monthShortStr(tm.Month), sizeof(month));
        StringUtil::sprintf(header, F("Cache-Control: max-age=600\r\nLast-Modified: %s, %d %s %d %02d:%02d:%02d GMT\r\n"),
            day, tm.Day, month, tmYearToCalendar(tm.Year),
            tm.Hour, tm.Minute, tm.Second);

        if (!strcasecmp(ext, "png")) {
            server.httpSuccess("image/png", header);
        }
        else if (!strcasecmp(ext, "js")) {
            server.httpSuccess("application/x-javascript", header);
        }
        else if (!strcasecmp(ext, "css")) {
            server.httpSuccess("text/css", header);
        }
        else if (!strcasecmp(ext, "txt")) {
            server.httpSuccess("text/plain; charset=utf-8", header);
        }
        else {
            server.httpSuccess("text/html; charset=utf-8", header);
        }
    }

    int readed;
    DEBUG("transfer");
    while ((readed = server.fh.read(server.requestBuffer, MY_HEATER_UI_BUFFER_LEN)) > 0) {
        server.write(server.requestBuffer, readed);
    }

    DEBUG("close");
    server.fh.close();
    DEBUG("stop");
}

static void webApiFileSystem(MyHeaterUI &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
    char value[MY_HEATER_UI_VALUE_LEN];
    char *p;

    DEBUG("webApiFileSystem");

    if (server.authCredentials != NULL && !server.checkCredentials(server.authCredentials)) {
        server.httpUnauthorized();
        return;
    }

    // root
    if (!server.sd->chdir()) {
        server.httpServerError();
        server.print("<h1>SD card failed</h1>");
        return;
    }

    if (type == WebServer::GET) {
        DEBUG("GET");
        FatFile dir;
        if (!getNextStrParam(server, &url_tail, "dir", value)) {
            strcpy(value, "/");
        }
        DEBUG("dir: %s", value);
        if (!dir.open(server.sd->vwd(), value, O_READ)) {
            DEBUG("ERROR - Can't open file!");
            server.httpServerError();
            return;
        }

        server.httpSuccess("application/json; charset=utf-8");
        server.print("{\"files\":[");
        FatFile file;
        bool isFirst = true;
        while (file.openNext(&dir, O_READ)) {
            if (!isFirst) {
                server.print(",");
            }
            else {
                isFirst = false;
            }

            server.print("{\"name\":\"");
            file.printName(&server);
            server.print("\"");

            server.print(",\"modifyDateTime\":\"");
            file.printModifyDateTime(&server);
            server.print("\"");

            server.print(",\"fileSize\":");
            file.printFileSize(&server);

            if (file.isDir()) {
                server.print(",\"isDir\":true");
            }
            else {
                server.print(",\"isDir\":false");
            }

            server.print("}");

            file.close();
        }
        server.print("]}");
        dir.close();
    }
    else if (type == WebServer::PUT) {
        if ((p = url_tail) && getNextStrParam(server, &p, "file", value)) {
            DEBUG("PUT file: %s", value);

            FatFile file;
            if (!file.open(value, O_WRITE | O_CREAT | O_TRUNC)) {
                server.httpServerError();
                return;
            }

            int ch;
            while ((ch = server.read()) != -1) {
                file.write(ch);
            }

            file.close();
        }
        else if ((p = url_tail) && getNextStrParam(server, &p, "dir", value)) {
            DEBUG("MKDIR dir: %s", value);

            if (!server.sd->mkdir(value)) {
                server.httpServerError();
                return;
            }
        }
        else {
            server.httpServerError();
            return;
        }
        server.httpSuccess();
    }
    else if (type == WebServer::DELETE) {
        FatFile file;
        while (getNextStrParam(server, &url_tail, "file", value)) {
            DEBUG("DELETE file: %s", value);
            if (file.open(value, O_READ)) {
                DEBUG("opened");
                if (file.isDir()) {
                    file.rmRfStar() && file.rmdir();
                    file.close();
                }
                else {
                    DEBUG("try to remove");
                    file.close();
                    if (file.open(value, O_READ | O_WRITE)) {
                        if (!file.remove()) {
                            DEBUG("Cannot remove file");
                        }
                        file.close();
                    }
                }
            }
            else {
                DEBUG("Cannot open file");
            }
        }
        server.httpSuccess();
    }
}

static void webIndex(MyHeaterUI &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
    char *url_path[2] = { (char *) "index.html", NULL };
    webFileAccess(server, type, url_path, url_tail, tail_complete);
}

static void webFileSystem(MyHeaterUI &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
    char *url_path[2] = { (char *) "fs.html", NULL };
    webFileAccess(server, type, url_path, url_tail, tail_complete);
}

static void webApiHeatersSet(MyHeaterUI &server, WebServer::ConnectionType type, char *url_tail, bool x) {
    long int          i;
    float             f;
    int               d, z;
    char              name[MY_HEATER_UI_NAME_LEN];
    char              value[MY_HEATER_UI_VALUE_LEN];
    char             *endptr, *v, *p;
    ScheduledHeater  *heater;

    if (server.authCredentials != NULL && !server.checkCredentials(server.authCredentials)) {
        server.httpUnauthorized();
        return;
    }

    while (server.readPOSTparam(name, MY_HEATER_UI_NAME_LEN, value, MY_HEATER_UI_VALUE_LEN)) {
        i = name[1] - '0';
        if (i < 0)
            goto INVALID_PARAMETERS;
        heater = (ScheduledHeater *) myHeater->heaters()->get(i);
        if (heater == NULL)
            goto INVALID_PARAMETERS;

        switch (name[0]) {
            case 'm':
                i = strtol(value, &endptr, 10);
                if (errno || endptr == value
                    || i < MY_HEATER_UI_MODE_MIN
                    || i > MY_HEATER_UI_MODE_MAX)
                    goto INVALID_PARAMETERS;

                heater->setMode((ScheduledHeater::Mode) i);
                break;

            case 't':
                f = strtod(value, &endptr);
                if (errno || endptr == value
                    || f < MY_HEATER_UI_TEMP_MIN
                    || f > MY_HEATER_UI_TEMP_MAX)
                    goto INVALID_PARAMETERS;
                heater->setTargetTemperature(f);
                break;

            case 's':
                d = name[2] - '0';
                z = name[3] - '0';
                if (d < 0 || d >= SCHEDULE_HEATER_MAX_DAYS
                    || z < 0 || z >= SCHEDULE_HEATER_MAX_ZONES)
                    goto INVALID_PARAMETERS;

                // start
                v = value;
                if ((p = strchr(v, '-')) == NULL)
                    goto INVALID_PARAMETERS;
                *p = '\0';
                i = strtol(v, &endptr, 10);
                if (errno || endptr == v
                    || i < MY_HEATER_UI_PERIOD_MIN
                    || i > MY_HEATER_UI_PERIOD_MAX)
                    goto INVALID_PARAMETERS;
                heater->schedule[d][z].start = i;
                v = p + 1;

                // end
                if ((p = strchr(v, '-')) == NULL)
                    goto INVALID_PARAMETERS;
                *p = '\0';
                i = strtol(v, &endptr, 10);
                if (errno || endptr == v
                    || i < MY_HEATER_UI_PERIOD_MIN
                    || i > MY_HEATER_UI_PERIOD_MAX)
                    goto INVALID_PARAMETERS;
                heater->schedule[d][z].end = i;
                v = p + 1;

                // setpoint
                f = strtod(v, &endptr);
                if (errno || endptr == v
                    || f < MY_HEATER_UI_TEMP_MIN
                    || f > MY_HEATER_UI_TEMP_MAX)
                    goto INVALID_PARAMETERS;
                heater->schedule[d][z].setpoint = f;
                break;

            default:
                goto INVALID_PARAMETERS;
        }
    }

    myHeater->heaters()->forceMaintain();

    myHeater->updateConfig(true);

    webApiHeatersGet(server, type, url_tail, x);

    return;

INVALID_PARAMETERS:
    server.httpFail();
}

MyHeaterUI::MyHeaterUI(const char *prefix, int port, const char *authCredentials, SdFat *sd): WebServer(prefix, port) {
    this->authCredentials = authCredentials;
    this->sd              = sd;

    this->_setDefaultCommand(&webIndex);

    this->_addCommand("api/heaters/get", &webApiHeatersGet);
    this->_addCommand("api/heaters/set", &webApiHeatersSet);
    this->_addCommand("api/fs", &webApiFileSystem);
    this->_addCommand("fs", &webFileSystem);
    this->_setUrlPathCommand(&webFileAccess);
}

void MyHeaterUI::maintain() {
    int len;

#ifdef ETHERNET_SERVER_HAS_CUSTOM_SOCKET
    for (uint8_t sock = MY_HEATER_UI_MIN_SOCK_NUM; sock < MY_HEATER_UI_MAX_SOCK_NUM; sock++) {
        len = MY_HEATER_UI_BUFFER_LEN;
        this->processConnection(sock, MyHeaterUI::requestBuffer, &len);
    }
#else
    this->processConnection(MyHeaterUI::requestBuffer, &len);
#endif
}

void MyHeaterUI::_addCommand(const char *verb, MyHeaterUICommand *cmd) {
    this->addCommand(verb, (Command *) cmd);
}

void MyHeaterUI::_setUrlPathCommand(MyHeaterUIUrlPathCommand *cmd) {
    this->setUrlPathCommand((UrlPathCommand *) cmd);
}

void MyHeaterUI::_setFailureCommand(MyHeaterUICommand *cmd) {
    this->setFailureCommand((Command *) cmd);
}

void MyHeaterUI::_setDefaultCommand(MyHeaterUICommand *cmd) {
    this->setDefaultCommand((Command *) cmd);
}
