#include "config.h"

volatile MQTTClient_deliveryToken deliveredtoken;          // volatile用于指示一个变量可能会被多个线程并发修改，告诉编译器在优化代码时不要对该变量进行优化操作。
void delivered(void *context, MQTTClient_deliveryToken dt) // 消息送达的回调
{
    // printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) // 收到消息的回调
{
    int i;
    char *payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char *)message->payload;
    for (i = 0; i < message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
int main(int argc, char *argv[])
{
    //-------------初始化mqtt
    cout<<"init mqtt..."<<endl;
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer; // 连接选项
    MQTTClient_message pubmsg = MQTTClient_message_initializer;                  // 消息
    MQTTClient_deliveryToken token;
    int rc;                                                                           // return code
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL); // 创建客户端实例
    conn_opts.username = USERNAME;                                                    // 用户名
    conn_opts.password = PASS;                                                        // 密码
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);    // 设置回调函数
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) // 连接服务器
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    // 连接成功
    cout<<"mqtt connect success."<<endl;
    cout<<"Subscribing to topic: "<<subTOPIC<<endl;
    MQTTClient_subscribe(client, subTOPIC, QOS); // 订阅
    deliveredtoken = 0;

    //--------------初始化mavsdk
    cout<<"init mavsdk..."<<endl;
    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection(conURL);

    if (connection_result != ConnectionResult::Success)
    {
        std::cerr << "Connection failed: " << connection_result << endl
                  << "connect url is: " << conURL << endl;
        exit(EXIT_FAILURE);
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system)
    {
        std::cerr << "Timed out waiting for system\n";
        return 1;
    }

    // Instantiate plugins.
    auto telemetry = Telemetry{system.value()};
    auto action = Action{system.value()};
    //待续...

    while (true)
    {
        pubmsg.payload = (char *)PAYLOAD;
        pubmsg.payloadlen = strlen(PAYLOAD);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        MQTTClient_publishMessage(client, pubTOPIC, &pubmsg, &token); // 发布消息
        // printf("Waiting for publication of %s\n" "on topic %s for client with ClientID: %s\n" "token is: %d\n", PAYLOAD, TOPIC, CLIENTID, token);
        sleep_for(seconds(2));
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
