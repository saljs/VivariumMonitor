#include "MockLib.h"
#include <cstdarg>

MockContainer global_mocks;

void
MockLib::Returns(std::string func, int count, ...)
{
  std::va_list args;
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    void* retval = va_arg(args, void*);
    returns_map[func].push(retval);
  }
  va_end(args);
}

void
MockLib::Expects(std::string arg, int count, ...)
{
  std::va_list args;
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    void* expval = va_arg(args, void*);
    expects_map[arg].push(expval);
  }
  va_end(args);
}

int
MockLib::Called(std::string func)
{
  if (calls_map.count(func)) {
    return calls_map[func];
  }
  return 0;
}

void
MockLib::Reset()
{
  returns_map.clear();
  expects_map.clear();
  calls_map.clear();
}

MockLib::MockLib()
{
  global_mocks.push_back(this);
}

MockLib::~MockLib()
{
  for (std::vector<MockLib*>::iterator i = global_mocks.begin();
       i != global_mocks.end();
       i++) {
    if (*i == this) {
      global_mocks.erase(i);
      break;
    }
  }
}

MockLib*
GetMock(std::string name)
{
  for (std::vector<MockLib*>::iterator i = global_mocks.begin();
       i != global_mocks.end();
       i++) {
    if ((*i)->GetName() == name) {
      return *i;
    }
  }
  return NULL;
}
