#include "mqtt.h"
#include "json.h"

const String mqtt_client_id               = String(ESP.getChipId());
const String mqtt_walvolas_topic          = "YOUR TOPIC/in/" + String(WALVOLA_ID); //MQTT topic on which walvola is listening for incoming commands
const String mqtt_controllers_topic       = "YOUR TOPIC/out/" + String(WALVOLA_ID); //MQTT topic on which walvola is posting reply to commands
const String mqtt_controllers_topic_debug = "YOUR TOPIC/out/" + String(WALVOLA_ID) + "/debug"; //MQTT topic for debug purposes. if MQTT_DEBUG then post to topic any log message
String all_walvoles_id                    = "allwalvoles"; //generic walvola id for broadcast messages
String all_controllers_id                 = "allcontollers"; //generic controller id for broadcast messages
String mqtt_server                        = "m20.cloudmqtt.com"; //MQTT broker sever. I use https://www.cloudmqtt.com/

PubSubClient mqtt_client(wifi_client, mqtt_server, MQTT_BROKER_PORT);    

//MQTT publish function for retained messages
void mqtt_publish_mex(String topic, String jmex, bool retained)
{
        log("MQTT published mex START");
        log(jmex);

        MQTT::Publish pub(topic, jmex);
        pub.set_retain(retained);
        if (mqtt_client.publish(pub)) {
                log("MQTT published mex SUCCESS");
        } else {
                log("MQTT published mex ERROR");
        }
}

//helper functions to get specific part of incoming MQTT mex
String mqtt_mex_src(){
        return getJSONvalue("src");
}

String mqtt_mex_dst()
{
        return getJSONvalue("dst");
}

String mqtt_mex_cmd()
{
        return getJSONvalue("cmd");
}

String mqtt_mex_type()
{
        return getJSONvalue("type");
}

String mqtt_mex_value()
{
        return getJSONvalue("value");
}

String mqtt_mex_voltage()
{
        return getJSONvalue("voltage");
}

String mqtt_mex_state()
{
        return getJSONvalue("state");
}

//GET_STATUS command handler
//provides voltage and status back to controller
void reply_CMD_GET_STATUS()
{
        log("processing CMD_GET_STATUS cmd........");
        log("Preparing json reply.........");
        String voltage;

        voltage = get_voltage();
        
        #if defined(WALVOLA_ROLE) || defined(WAC_ROLE)
                String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_GET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + walvola_status + "\"}}";
        #endif

        #ifdef IRB_ROLE
                String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_GET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + relay_status + "\"}}";
        #endif

        log(jreply);

        mqtt_client.publish(mqtt_controllers_topic, jreply);
        log("CMD_GET_STATUS reply sending SUCCESS.........");
}

//SET_STATUS command handler
//change status of walvola/irb based on controlelr command
void reply_CMD_SET_STATUS(String mqtt_value)
{
        log("processing CMD_SET_STATUS cmd........");
        log("Preparing json reply.........");
        String voltage;

        voltage = get_voltage();
        String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_SET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"TRANSITION\"}}";
 
        log(jreply);
        mqtt_publish_mex(mqtt_controllers_topic, jreply, true);
 
        #if defined(WALVOLA_ROLE) || defined(WAC_ROLE)
                log("SET WALVOLA START");
                set_walvola(mqtt_value);
                log("SET WALVOLA END");
        #endif

        #ifdef IRB_ROLE
                set_irb(mqtt_value);
        #endif

        voltage = get_voltage();
        String jreply2 = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_SET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + mqtt_value + "\"}}";
        log(jreply2);
        mqtt_publish_mex(mqtt_controllers_topic, jreply2, true);

        log("CMD_SET_STATUS reply sending SUCCESS.........");
}

//program UPDATE command handler
//if OTA_MODE defined put the ESP into OTA mode for program upgrade
void reply_CMD_UPDATE()
{
        String voltage;
        log("processing CMD_UPDATE cmd........");
        log("Preparing json reply.........");
        update_mode = true;

        #ifdef OTA_MODE
               OTA_init();
        #endif

        voltage = get_voltage();

        String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_UPDATE + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + String(WALVOLA_OTA) + "\"}}";
        log(jreply);

        mqtt_client.publish(mqtt_controllers_topic, jreply);
        log("CMD_UPDATE reply sending SUCCESS.........");
}

//GET_INFO command handler
//provide general device info (may be this is a kind of replice of GET_STATUS).I might remove later on
void reply_CMD_GET_INFO()
{
        String ip = WiFi.localIP().toString();
        String voltage = get_voltage();

        log("processing CMD_GET_INFO cmd........");
        log("Preparing json reply.........");

        //debug
        //tgram_sendmex("GET INFO");
        
        String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_GET_INFO + "\",\"value\": {\"version\":\"" + String(VERSION) + "DEBUG[" + String(DEBUG) + "]" + "\", {\"voltage\":\"" + voltage + "\", \"state\":\"" + walvola_status + "\", \"IP ADDRESS\":\"" + ip + "\"}}";
        log(jreply);

        mqtt_client.publish(mqtt_controllers_topic, jreply);
        log("CMD_GET_INFO reply sending SUCCESS.........");
}

//publish log message top debug topic (mqtt_controllers_topic_debug)
void mqtt_log(String mex)
{
        String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"debug\" : \"" + mex + "\", ";
        mqtt_client.publish(mqtt_controllers_topic_debug, jreply);
}

//incoming MQTT message dispatcher
void mqtt_mex_process(String mqtt_cmd, String mqtt_value)
{
        //only known commands are processed. everythign else is discarded
        log("Processing received mqtt message........");
        log("cmd received:");
        log(mqtt_cmd);
        log("value received:");
        log(mqtt_value);

        if (mqtt_cmd == CMD_GET_STATUS) {
               reply_CMD_GET_STATUS();
        } else if (mqtt_cmd == CMD_SET_STATUS) {
                reply_CMD_SET_STATUS(mqtt_value);
        } else if (mqtt_cmd == CMD_UPDATE) {
                reply_CMD_UPDATE();
        } else if (mqtt_cmd == CMD_GET_INFO) {
                reply_CMD_GET_INFO();
        } else {
                log("Unknow command received........");
        }
}

//MQTT callback function invoked for every MQTT received message on a subscribed topic
void mqtt_callback(const MQTT::Publish& pub)
{
        log("MQTT received message:");
        log(pub.payload_string());
        //{u'src': u'app1234', u'dst': u'allwalvoles', u'cmd': u'GET_STATUS', u'type': u'REQUEST', u'value': u'0'}

        //convert the incoming MQTT message to JSON
        if (mqtt2JSONkvs(pub.payload_string())) {
                //process only broadcast messages or the one specifically addressed to a given device
                if (mqtt_mex_dst() == all_walvoles_id || mqtt_mex_dst() == WALVOLA_ID) {
                log("Receiving a message addressed to me..........");

                String mqtt_cmd = mqtt_mex_cmd();
                String mqtt_value = mqtt_mex_value();

                //need to clear the last retained message in input queue
                //this is necessary to clear the buffer of incoming message
                log("SENDING MQTT EMPTY MESSAGE");
                mqtt_publish_mex(mqtt_walvolas_topic, "", true);

                log("MQTT PROCESS MEX START");
                mqtt_mex_process(mqtt_cmd, mqtt_value);
                } else {
                        log("Receiving a message NOT addressed to me, so discarding it..........");
                }
        }
}

//MQTT initialization
int MQTT_init(boolean topic_subscribe)
{
        log("Initializing MQTT communication.........");
        log(mqtt_client_id);
        log(mqtt_walvolas_topic);

        mqtt_client.set_callback(mqtt_callback);
        mqtt_client.set_max_retries(255);
        log("mqtt_callback succesfully set up.........");

        //here we connect to MQTT broker and we increase the keepalive for more reliability
        if (mqtt_client.connect(MQTT::Connect(mqtt_client_id).set_keepalive(90).set_auth(String(MQTT_UNAME), String(MQTT_PASSW)))) {
                log("Connection to MQTT broker SUCCESS..........");

        //MQTT subscribe to walvola topic (client -> walvola)
                if (topic_subscribe) {
                        if (mqtt_client.subscribe(mqtt_walvolas_topic)) {
                                log("Subscription to MQTT topic [" + mqtt_walvolas_topic + "] SUCCESS.........");
                                log("MQTT Client loop init");
        
                                //now that we are sucesfully subscriber to our incoming topic (mqtt_walvolas_topic)
                                //we can check whether there is a message waiting for our processing
                                //as after subscription the message is not immediately avaialble let's retry few times to be sure we are not missing any available message
                                for (int i = 0; i < 5; i++) {
                                        mqtt_client.loop();
                                        log("MQTT client loop");
                                delay(100);
                                }
                                log("MQTT Client loop finish");
                        } else {
                                log("MQTT unable to subscribe to [" + mqtt_walvolas_topic + "] ERROR.........");
                                return false;
                        }
                }
        } else {
                log("Connection to MQTT broker ERROR..........");
        }

        //we always provide back to controller a GET_STATUS REPLY message
        //publish to controllers topic (walvola -> client) the walvola status
        String voltage = get_voltage();

        #if defined(WALVOLA_ROLE) || defined(WAC_ROLE)
                String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_GET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + walvola_status + "\"}}"; 
        #endif

        #ifdef IRB_ROLE
                String jreply = "{\"time\":\"" + walvola_time + "\", \"src\":\"" + String(WALVOLA_ID) + "\", \"dst\": \"allcontollers\", \"label\" : \"" + String(WALVOLA_LABEL) + "\", \"type\": \"REPLY\", \"cmd\":\"" + CMD_GET_STATUS + "\",\"value\": {\"voltage\":\"" + voltage + "\", \"state\":\"" + relay_status + "\"}}";
        #endif  
        mqtt_publish_mex(mqtt_controllers_topic, jreply, true);

        return mqtt_client.connected();
}

void mqtt_disconnect()
{
        log("Disconnecting MQTT");
        mqtt_client.disconnect();
}

