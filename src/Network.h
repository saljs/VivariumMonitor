/*
 * Network.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"
#include <time.h>

/*
 * Interface to network components.
 */
class Network
{
public:
  void init(VivariumMonitorConfig* config, Url update_endpoint);
  void update_firmware(time_t now);
  void serve_web_interface();
  void post_stats(SensorData& readings,
                  byte digital_1,
                  byte digital_2,
                  byte analog);

private:
  ViviariumMonitorConfig* monitor_config = NULL;
  Url update_url;
  SensorData last_collected;
  time_t last_fw_check = 0;
  time_t last_sent = 0;
};

/*
 * HTTP request timeout
 */
#define HTTP_TIMEOUT 8000

/*
 * Interval to check for firmware updates
 */
#define FIRMWARE_CHECK_SECONDS 14400

/*
 * Size of JSON text buffer
 */
#define JSONBUF_SIZE 166

/*
 * Web server strings
 */
#define HTTP_404_RESPONSE                                                      \
  "\
HTTP/1.1 404 NOT FOUND\r\n\
Content-type:text/plain\r\n\
Content-Length:12\r\n\
Connection:close\r\n\r\n\
Not found.\r\n"

#define HTTP_WEB_ROOT_HEAD                                                     \
  "\
HTTP/1.1 200 OK\r\n\
Content-type:text/html\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html>\r\n\
<html>\r\n\
    <head>\r\n\
        <title>Vivarium Monitor Web Interface</title>\r\n\
    </head>\r\n\
    <body>\r\n\
        <h3>Vivarium Monitor Web Interface</h3>\r\n\
        <p>Node information:</p>\r\n\
        <ul>\r\n\
            <li><b>Firmare version:</b> " FIRMWARE_VERSION "</li>\r\n"

#define HTTP_WEB_ROOT_FOOTER                                                   \
  "\
        </ul>\r\n\
        <a href=\"/rs\">\r\n\
            <button>Reset device</button>\r\n\
        </a>\r\n\
    </body>\r\n\
</html>\r\n"

#define HTTP_WEB_RS                                                            \
  "\
HTTP/1.1 200 OK\r\n\
Content-type:text/html\r\n\
Content-Length:250\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html>\r\n\
<html>\r\n\
    <head>\r\n\
        <title>Vivarium Monitor Web Interface</title>\r\n\
    </head>\r\n\
    <body>\r\n\
        <h3>Vivarium Monitor Web Interface</h3>\r\n\
        <p>Node has been reset to default configuration.</p>\r\n\
    </body>\r\n\
</html>\r\n"

#endif
