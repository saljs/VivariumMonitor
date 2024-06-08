#include "StreamUtils.h"
#include <cctype>
#include <cstdarg>
#include <cstring>

std::string global_input_stream = "";
int global_input_stream_loc = 0;

std::string
GetRemaining()
{
  int len = global_input_stream.length() - global_input_stream_loc;
  return global_input_stream.substr(global_input_stream_loc, len);
}

ReadBufferingStream::ReadBufferingStream(Stream& stream, int bufLen) {}
int
ReadBufferingStream::available()
{
  return global_input_stream_loc < global_input_stream.length();
}
int
ReadBufferingStream::find(const char* match)
{
  int pos = global_input_stream.find(match);
  if (pos >= global_input_stream_loc) {
    global_input_stream_loc = pos + strlen(match);
    return true;
  }
  return false;
}
int
ReadBufferingStream::read()
{
  if (global_input_stream_loc < global_input_stream.length()) {
    return global_input_stream.at(global_input_stream_loc++);
  }
  return -1;
}
int
ReadBufferingStream::parseInt()
{
  // Doesn't handle negative numbers
  int ret;
  while (!isdigit(global_input_stream.at(global_input_stream_loc))) {
    global_input_stream_loc++;
  }
  ret = stoi(GetRemaining());
  while (isdigit(global_input_stream.at(global_input_stream_loc))) {
    global_input_stream_loc++;
  }
  return ret;
}
size_t
ReadBufferingStream::readBytes(char* buffer, size_t arg_1)
{
  if (global_input_stream.length() - global_input_stream_loc > arg_1) {
    arg_1 = global_input_stream.length() - global_input_stream_loc;
  }
  for (int i = 0; i < arg_1; i++) {
    buffer[i] = (char)read();
  }
  return arg_1;
}
size_t
ReadBufferingStream::readBytesUntil(char arg_1, char* buffer, size_t arg_2)
{
  int i = 0, c;
  if (global_input_stream.length() - global_input_stream_loc > arg_2) {
    arg_2 = global_input_stream.length() - global_input_stream_loc;
  }
  do {
    c = read();
    buffer[i] = (char)c;
    i++;
  } while (i < arg_2 && c != arg_1);
  return i;
}
String
ReadBufferingStream::readString()
{
  std::string ret = GetRemaining();
  global_input_stream_loc = global_input_stream.length();
  return String(ret.c_str());
}

WriteBufferingStream::WriteBufferingStream(Stream& stream, int bufLen)
{
  underlay = &stream;
}

size_t
WriteBufferingStream::printf(const char* format, ...)
{
  char buf[256];
  va_list args;

  va_start(args, format);
  vsnprintf(buf, 256, format, args);
  va_end(args);
  return underlay->print(buf);
}
size_t
WriteBufferingStream::write(const uint8_t* data, size_t arg_1)
{
  return underlay->write(data, arg_1);
}
size_t
WriteBufferingStream::write(uint8_t arg_1)
{
  return underlay->write(arg_1);
}
size_t
WriteBufferingStream::write(const char* data, size_t arg_1)
{
  return underlay->write(data, arg_1);
}

void
SetGlobalInputStream(std::string text)
{
  global_input_stream = text;
  global_input_stream_loc = 0;
}
