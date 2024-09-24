# 顶层CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(mqttclient VERSION 0.1)

# 查找eclipse-paho-mqtt库
find_package(PkgConfig REQUIRED)
find_package(MAVSDK REQUIRED)

# 添加可执行文件
add_executable(mqttclient)
target_sources(mqttclient PRIVATE mqtt_client.cpp)
# 添加头文件
## Paho MQTT C Client Library
target_include_directories(mqttclient PRIVATE /usr/local/include/*.h)
# 链接OpenCV库
## Paho MQTT C Client Library
target_link_libraries(mqttclient /usr/local/lib/libpaho-mqtt3a.so /usr/local/lib/libpaho-mqtt3as.so /usr/local/lib/libpaho-mqtt3c.so /usr/local/lib/libpaho-mqtt3cs.so)