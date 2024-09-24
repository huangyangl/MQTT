#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <iostream>
#include <future>
#include <memory>

#define ADDRESS "tcp://47.113.112.74:1883"
#define CLIENTID "shd01"
#define USERNAME "shd01"
#define PASS "shd01"
#define pubTOPIC "copter"
#define subTOPIC "cmd"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L
#define conURL "serial:///dev/ttyACM0"

using std::chrono::seconds;
using std::this_thread::sleep_for;
using namespace std;