#ifndef MOCKLIB_H
#define MOCKLIB_H

#include <iostream>

#include <cassert>
#include <cstdint>
#include <format>
#include <map>
#include <stack>
#include <string>
#include <vector>

class MockLib
{
public:
  MockLib();
  ~MockLib();
  virtual std::string GetName() { return "default"; }
  void Returns(std::string func, int count, ...);
  void Expects(std::string arg, int count, ...);
  int Called(std::string func);
  void Reset();
  std::map<std::string, std::stack<void*>> returns_map;
  std::map<std::string, std::stack<void*>> expects_map;
  std::map<std::string, int> calls_map;
};

typedef std::vector<MockLib*> MockContainer;

MockLib*
GetMock(std::string name);

/*
 * Defines a macros for quickly creating mock functons.
 */
#define MOCK_COUNT                                                             \
  if (calls_map.count(__func__)) {                                             \
    calls_map[__func__]++;                                                     \
  } else {                                                                     \
    calls_map[__func__] = 1;                                                   \
  }

#define MOCK_RETURN(RET_T)                                                     \
  if (returns_map.count(__func__) && !returns_map[__func__].empty()) {         \
    RET_T* ret = (RET_T*)returns_map[__func__].top();                          \
    returns_map[__func__].pop();                                               \
    return *ret;                                                               \
  }

#define MOCK_ARG_CHECK(ARG, ARG_T, VAL)                                        \
  key = std::format("{0}.{1}", __func__, ARG);                                 \
  if (expects_map.count(key) && !expects_map[key].empty()) {                   \
    ARG_T* exp = (ARG_T*)expects_map[key].top();                               \
    expects_map[key].pop();                                                    \
    if (*exp != VAL) {                                                         \
      std::cout << std::format(                                                \
        "ERROR: {0} expected {1}, got {2}\n", key, *exp, VAL);                 \
      assert(false);                                                           \
    }                                                                          \
  }

#define MOCK_FUNC_V0 MOCK_COUNT

#define MOCK_FUNC_R0(RET_T)                                                    \
  MOCK_COUNT                                                                   \
  MOCK_RETURN(RET_T)

#define MOCK_FUNC_V1(ARG1_T)                                                   \
  std::string key;                                                             \
  MOCK_COUNT                                                                   \
  MOCK_ARG_CHECK("arg_1", ARG1_T, arg_1)

#define MOCK_FUNC_R1(RET_T, ARG1_T)                                            \
  MOCK_FUNC_V1(ARG1_T)                                                         \
  MOCK_RETURN(RET_T)

#define MOCK_FUNC_V2(ARG1_T, ARG2_T)                                           \
  std::string key;                                                             \
  MOCK_COUNT                                                                   \
  MOCK_ARG_CHECK("arg_1", ARG1_T, arg_1)                                       \
  MOCK_ARG_CHECK("arg_2", ARG2_T, arg_2)

#define MOCK_FUNC_R2(RET_T, ARG1_T, ARG2_T)                                    \
  MOCK_FUNC_V2(ARG1_T, ARG2_T)                                                 \
  MOCK_RETURN(RET_T)

#define MOCK_FUNC_V3(ARG1_T, ARG2_T, ARG3_T)                                   \
  std::string key;                                                             \
  MOCK_COUNT                                                                   \
  MOCK_ARG_CHECK("arg_1", ARG1_T, arg_1)                                       \
  MOCK_ARG_CHECK("arg_2", ARG2_T, arg_2)                                       \
  MOCK_ARG_CHECK("arg_3", ARG3_T, arg_3)

#define MOCK_FUNC_R3(RET_T, ARG1_T, ARG2_T, ARG3_T)                            \
  (ARG1_T, ARG2_T, ARG3_T) MOCK_RETURN(RET_T)

#define MOCK_FUNC_V4(ARG1_T, ARG2_T, ARG3_T, ARG4_T)                           \
  std::string key;                                                             \
  MOCK_COUNT                                                                   \
  MOCK_ARG_CHECK("arg_1", ARG1_T, arg_1)                                       \
  MOCK_ARG_CHECK("arg_2", ARG2_T, arg_2)                                       \
  MOCK_ARG_CHECK("arg_3", ARG3_T, arg_3)                                       \
  MOCK_ARG_CHECK("arg_4", ARG4_T, arg_4)

#define MOCK_FUNC_R4(RET_T, ARG1_T, ARG2_T, ARG3_T, ARG4_T)                    \
  MOCK_FUNC_V4(ARG1_T, ARG2_T, ARG3_T, ARG4_T)                                 \
  MOCK_RETURN(RET_T)

#define MOCK_FUNC_V5(ARG1_T, ARG2_T, ARG3_T, ARG4_T, ARG5_T)                   \
  std::string key;                                                             \
  MOCK_COUNT                                                                   \
  MOCK_ARG_CHECK("arg_1", ARG1_T, arg_1)                                       \
  MOCK_ARG_CHECK("arg_2", ARG2_T, arg_2)                                       \
  MOCK_ARG_CHECK("arg_3", ARG3_T, arg_3)                                       \
  MOCK_ARG_CHECK("arg_4", ARG4_T, arg_4)                                       \
  MOCK_ARG_CHECK("arg_5", ARG5_T, arg_5)

#define MOCK_FUNC_R5(RET_T, ARG1_T, ARG2_T, ARG3_T, ARG4_T, ARG5_T)            \
  MOCK_FUNC_V5(ARG1_T, ARG2_T, ARG3_T, ARG4_T, ARG5_T)                         \
  MOCK_RETURN(RET_T)

#endif
