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
 * Primary interface highlight color
 */
#define CSS_PRIMARY_COLOR "#00c060"

/*
 * Web server strings
 */
const char http_404_response[] PROGMEM = "HTTP/1.0 404 NOT FOUND\r\n\
Content-type:text/plain\r\n\
Content-Length:12\r\n\
Connection:close\r\n\r\n\
Not found.\r\n";

const char http_root_header[] PROGMEM =
  "HTTP/1.0 200 OK\r\n\
Content-type:text/html\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html><html><head><title>Vivarium Monitor Web Interface</title><meta content=\"width=device-width,initial-scale=1,user-scalable=no\" name=viewport /><style>body{font-family:verdana;display:flex;flex-direction:column;align-items:center;color:#3e3e3e;background-color:#f4f4f4}.wrap{align-content:center;padding:2rem}.info{text-align:left;font-size:1.15rem;display:inline-block;min-width:260px;max-width:500px}.info>ul{line-height:1.6rem}input[type=submit]{border:0;width:80%;margin-top:1.2rem;margin-left:10%;background-color:" CSS_PRIMARY_COLOR
  ";color:#f4f4f4;line-height:2.4rem;font-size:1.2rem;border-radius:.3rem;"
  "cursor:pointer;opacity:1;transition:.3s}input[type=submit]:hover{opacity:.5}"
  ".protect::after{content:\"\\26A0\";position:relative;left:-2rem;line-height:"
  "2.4rem;font-size:1.5rem;color:#f4f4f4}</"
  "style><script>document.addEventListener(\"DOMContentLoaded\",function(){let "
  "t=document.getElementsByClassName(\"protect\");for(let "
  "e=0;e<t.length;e++)t[e].addEventListener(\"submit\",function(t){return "
  "t.preventDefault(),confirm(\"Are you sure? This action cannot be "
  "undone.\")&&this.submit(),!1});let "
  "n=document.getElementsByClassName(\"time\");for(let o=0;o<n.length;o++){let "
  "l=new "
  "Date(1e3*parseInt(n[o].textContent));n[o].textContent=l.toLocaleString()}});"
  "</script></head><body><div class=wrap><div class=info><h2>Vivarium Monitor "
  "Web Interface</h2><h3>Node information:</h3><ul>";

const char http_root_footer[] PROGMEM =
  "</ul></div><div class=actions><form action=/rb><input type=submit "
  "value=\"Reboot device\"/></form><form action=/rs class=protect><input "
  "type=submit value=\"Reset device\"/></form></div></div></body></html>\r\n";

const char http_page_rs[] PROGMEM = "HTTP/1.0 200 OK\r\n\
Content-type:text/html\r\n\
Content-Length:582\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html><html><head><title>Vivarium Monitor Web Interface</title><meta content=\"width=device-width,initial-scale=1,user-scalable=no\"name=viewport /><style>body{font-family:verdana;display:flex;flex-direction:column;align-items:center;color:#3e3e3e;background-color:#f4f4f4}.wrap{align-content:center;padding:2rem}.info{text-align:left;font-size:1.15rem;display:inline-block;min-width:260px;max-width:500px}</style></head><body><div class=wrap><div class=info><h2>Vivarium Monitor Web Interface</h2><h3>Node has been reset to default configuration.</h3></div></body></html>\r\n";

const char http_page_rb[] PROGMEM = "HTTP/1.0 200 OK\r\n\
Content-type:text/html\r\n\
Content-Length:558\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html><html><head><title>Vivarium Monitor Web Interface</title><meta content=\"width=device-width,initial-scale=1,user-scalable=no\"name=viewport /><style>body{font-family:verdana;display:flex;flex-direction:column;align-items:center;color:#3e3e3e;background-color:#f4f4f4}.wrap{align-content:center;padding:2rem}.info{text-align:left;font-size:1.15rem;display:inline-block;min-width:260px;max-width:500px}</style></head><body><div class=wrap><div class=info><h2>Vivarium Monitor Web Interface</h2><h3>Node has been reboot.</h3></div></body></html>\r\n";
#endif
