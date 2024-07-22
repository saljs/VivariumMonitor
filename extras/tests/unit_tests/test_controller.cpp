#include <Hardware.h>
#include <MockLib.h>
#include <cassert>

void
test_clean_and_dirty_write()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock Wire library
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  testHarness.init(&config);
  assert(MockWireLib->Called("write") == 0);

  // Test that writing clean outputs doesn't reach hardware
  testHarness.write_outputs();
  assert(MockWireLib->Called("write") == 0);

  // Change a value and check again
  testHarness.set_analog(50);
  testHarness.write_outputs();
  int bytesOut = MockWireLib->Called("write");
  assert(bytesOut > 0);

  // Assert multiple tries to write value out
  testHarness.write_outputs();
  assert(MockWireLib->Called("write") > bytesOut);

  // Assert that we eventually stop sending
  for (int i = 0; i < 10; i++) {
    testHarness.write_outputs();
    int written = MockWireLib->Called("write");
    if (written == bytesOut) {
      // success
      return;
    }
    bytesOut = written;
  }
  // If the loop goes all the way through the test has failed
  assert(false);
}

void
test_correct_checksum()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock Wire library
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  testHarness.init(&config);

  // Set some values
  testHarness.set_analog(50);
  testHarness.set_digital_2(1);
  testHarness.set_digital_1(1);

  // Check that the corrct values are sent
  int b1 = 50, b2 = 35;
  MockWireLib->Expects("write.arg_1", 2, &b2, &b1);
  testHarness.write_outputs();
  assert(MockWireLib->Called("write") > 0);
}

int
main(void)
{
  test_clean_and_dirty_write();
  test_correct_checksum();
  return 0;
}
