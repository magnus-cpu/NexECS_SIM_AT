#ifndef AT_CMD_H
#define AT_CMD_H

#define AT_CMD_BASIC                 "AT"
#define AT_CMD_ECHO_OFF              "ATE0"
#define AT_CMD_ECHO_ON               "ATE1"

#define AT_CMD_CHECK_SIM             "AT+CPIN?"
#define AT_CMD_SIGNAL_QUALITY        "AT+CSQ"
#define AT_CMD_OPERATOR              "AT+COPS?"
#define AT_CMD_NETWORK_STATUS        "AT+CREG?"

#define AT_CMD_SMS_TEXT_MODE         "AT+CMGF=1"
#define AT_CMD_SMS_SEND              "AT+CMGS="

#define AT_CMD_PDP_CONTEXT           "AT+CGDCONT=1,\"IP\",\"%s\""
#define AT_CMD_PDP_ACTIVATE          "AT+CGACT=1,1"
#define AT_CMD_PDP_DEACTIVATE        "AT+CGACT=0,1"

#define AT_CMD_MQTT_START            "AT+CMQTTSTART"
#define AT_CMD_MQTT_STOP             "AT+CMQTTSTOP"


// for mqtt using sim7600
#define qtt_start "AT+CMQTTSTART"  //Start MQTT service
#define qtt_stop "AT+CMQTTSTOP" // Stop MQTT service
#define qtt_acquire_cl "AT+CMQTTACCQ" // Acquire a MQTT client
#define qtt_release_cl  "AT+CMQTTREL" //Release a MQTT client
#define qtt_ssl "AT+CMQTTSSLCFG" //Set the SSL context
#define qtt_willTopic "AT+CMQTTWILLTOPIC" //Input the topic of will message
#define qtt_willMsg "AT+CMQTTWILLMSG" // Input the will message
#define qtt_connect "AT+CMQTTCONNECT" //Connect to a MQTT server
#define qtt_disconnect"AT+CMQTTDISC"// Disconnect from server
#define qtt_inputPubTopic "AT+CMQTTTOPIC " //Input the publish message topic
#define qtt_inputPubMsg"AT+CMQTTPAYLOAD" //Input the publish message body
#define qtt_pubMsg"AT+CMQTTPUB" //Publish a message to server
#define qtt_inputSubTopic "AT+CMQTTSUBTOPIC" //Input a subscribe message topic
#define qtt_subMsg "AT+CMQTTSUB" // Subscribe a message to server
#define qtt_inputUnSubTopic "AT+CMQTTUNSUBTOPIC" //Input a unsubscribe message topic
#define qtt_unSubMsg "AT+CMQTTUNSUB" // Unsubscribe a message to server
#define qtt_config "AT+CMQTTCFG" // Configure the MQTT Contex


#endif
