#include "AntaresESP32MQTT.h"

WiFiClient espClient;
PubSubClient client(espClient);

AntaresESP32MQTT::AntaresESP32MQTT(String accessKey) {
    _accessKey = accessKey;
}

void AntaresESP32MQTT::setMqttServer() {
    if(WiFi.status() != WL_CONNECTED) {
        printDebug("[ANTARES] Unable to connect to MQTT server.\n");
    }
    else {
        printDebug("[ANTARES] Setting MQTT server \"" + String(_mqttServer) + "\" on port " + String(_mqttPort) + "\n");
        client.setServer(_mqttServer, _mqttPort);
    }

}

void AntaresESP32MQTT::checkMqttConnection() {
    _subscriptionTopic = "/oneM2M/resp/antares-cse/" + _accessKey + "/json";

    if(!client.connected()) {
        while(!client.connected()) {
            printDebug("[ANTARES] Attempting MQTT connection...\n");

            String clientId = "ESP32-" + _accessKey;

            char clientIdChar[clientId.length() + 1];
            clientId.toCharArray(clientIdChar, clientId.length() + 1);

            if(client.connect(clientIdChar)) {
                printDebug("[ANTARES] Connected! Client ID:");
                printDebug(clientIdChar);
                printDebug("\n");
                char subscriptionTopicChar[_subscriptionTopic.length() + 1];
                _subscriptionTopic.toCharArray(subscriptionTopicChar, _subscriptionTopic.length() + 1);

                Serial.println();
                Serial.print("[ANTARES] Subscribe Topic: ");
                Serial.println(subscriptionTopicChar);

                client.publish(subscriptionTopicChar, "connect!");
                client.subscribe(subscriptionTopicChar);
            }
            else {
                printDebug("[ANTARES] Failed, rc=" + String(client.state()) + ", Will try again in 5 secs.\n");
                delay(5000);
            }
        }
    }
    client.loop();
}

bool AntaresESP32MQTT::wifiConnection(String SSID, String wifiPassword) {
    char ssidChar[sizeof(SSID)];
    char wifiPasswordChar[sizeof(wifiPassword)];

    SSID.toCharArray(ssidChar, sizeof(SSID));
    wifiPassword.toCharArray(wifiPasswordChar, sizeof(wifiPassword));

    int count = 0;
    _wifiSSID = ssidChar;
    _wifiPass = wifiPasswordChar;

    WiFi.begin(_wifiSSID, _wifiPass);
    printDebug("[ANTARES] Trying to connect to " + SSID + "...\n");

    for (count=0;count<20;count++)
    {
      delay(500);
      printDebug(".");
    }

    if(WiFi.status() != WL_CONNECTED) {
        printDebug("[ANTARES] Could not connect to " + SSID + ".\n");
        return false;
    }
    else {
        WiFi.setAutoReconnect(true);
        printDebug("\n[ANTARES] WiFi Connected!\n");
        printDebug("[ANTARES] IP Address: " + ipToString(WiFi.localIP()) + "\n");
        return true;
    }
}

void AntaresESP32MQTT::add(String key, int value) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonDataString);
    object[key] = value;
    String newInsert;
    object.printTo(newInsert);
    _jsonDataString = newInsert;
}

void AntaresESP32MQTT::add(String key, float value) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonDataString);
    object[key] = value;
    String newInsert;
    object.printTo(newInsert);
    _jsonDataString = newInsert;
}

void AntaresESP32MQTT::add(String key, double value) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonDataString);
    object[key] = value;
    String newInsert;
    object.printTo(newInsert);
    _jsonDataString = newInsert;
}

void AntaresESP32MQTT::add(String key, String value) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonDataString);
    object[key] = value;
    String newInsert;
    object.printTo(newInsert);
    _jsonDataString = newInsert;
}

void AntaresESP32MQTT::printData() {
    printDebug("[ANTARES] " + _jsonDataString + "\n");
}

void AntaresESP32MQTT::printDebug(String text) {
    if(_debug) {
        Serial.print(text);
    }
}

void AntaresESP32MQTT::publish(String projectName, String deviceName) {
    String topic = "/oneM2M/req/" + _accessKey + "/antares-cse/json";
    String finalData;

    if(_debug) {
        DynamicJsonBuffer jsonBuffer;
        JsonObject& object = jsonBuffer.parseObject(_jsonDataString);
        Serial.print("[ANTARES] Publish Topic: ");Serial.println(topic);
        printDebug("[ANTARES] PUBLISH DATA:\n\n");
        object.prettyPrintTo(Serial);
        Serial.println("\n");
    }

    _jsonDataString.replace("\"", "\\\"");


    finalData += "{";
    finalData += "\"m2m:rqp\": {";
    finalData += "\"fr\": \"" + _accessKey +"\",";
    finalData += "\"to\": \"/antares-cse/antares-id/" + projectName + "/" + deviceName + "\",";
    finalData += "\"op\": 1,";
    finalData += "\"rqi\": 123456,";
    finalData += "\"pc\": {";
    finalData += "\"m2m:cin\": {";
    finalData += "\"cnf\": \"message\",";
    finalData += "\"con\": \""+ _jsonDataString + "\"";
    finalData += "}";
    finalData += "},";
    finalData += "\"ty\": 4";
    finalData += "}";
    finalData += "}";

    // DynamicJsonBuffer jsonBuffer;
    // JsonObject& object = jsonBuffer.parseObject(finalData);
    // object.prettyPrintTo(Serial);

    char finalDataChar[finalData.length() + 1];
    char topicChar[topic.length() + 1];

    finalData.toCharArray(finalDataChar, finalData.length() + 1);
    topic.toCharArray(topicChar, topic.length() + 1);

    _jsonDataString = "{}";

    client.publish(topicChar, finalDataChar);
}

int AntaresESP32MQTT::getInt(String key) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonSubDataString);
    return object[key];
}

float AntaresESP32MQTT::getFloat(String key) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonSubDataString);
    return object[key];
}

double AntaresESP32MQTT::getDouble(String key) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonSubDataString);
    return object[key];
}

String AntaresESP32MQTT::getString(String key) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(_jsonSubDataString);
    return object[key];
}

void AntaresESP32MQTT::setCallback(std::function<void(char*, uint8_t*, unsigned int)> callbackFunc) {
    client.setCallback(callbackFunc);
}

bool AntaresESP32MQTT::setDebug(bool trueFalse) {
    _debug = trueFalse;
}

String AntaresESP32MQTT::get(char* topic, byte* payload, unsigned int length) {
    _receivedTopic = String(topic);

    String payloadString;
    for(int i = 0; i < length; i++) {
        payloadString += char(payload[i]);
    }

    DynamicJsonBuffer jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(payloadString);
    String parsedString = object["m2m:rsp"]["pc"]["m2m:cin"]["con"];
    _jsonSubDataString = parsedString;

    return _jsonSubDataString;
}

String AntaresESP32MQTT::getTopic() {
    return _receivedTopic;
}

String AntaresESP32MQTT::getPayload() {
    return _jsonSubDataString;
}

String AntaresESP32MQTT::ipToString(IPAddress ip) {
    String s="";
    for (int i=0; i<4; i++)
      s += i  ? "." + String(ip[i]) : String(ip[i]);
    return s;
}

void AntaresESP32MQTT::setSubscriptionTopic() {
    _subscriptionTopic = "/oneM2M/req/" + _accessKey + "/antares-cse/json";
}
