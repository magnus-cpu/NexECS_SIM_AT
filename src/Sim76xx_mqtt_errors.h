#ifndef SIM76XX_MQTT_ERRORS_H
#define SIM76XX_MQTT_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @file SIM76xx_mqtt_errors.h
     * @brief MQTT(S) <err> result codes for SIM7500 / SIM7600 series
     *
     * Source:
     * SIM7500_SIM7600 Series AT Command Manual V3.00
     * Section 16.3 – Summary of result codes for MQTT(S)
     *
     * NOTE:
     * These error codes are returned by AT+CMQTTxxx commands
     * such as AT+CMQTTCONNECT, AT+CMQTTPUB, AT+CMQTTSUB, etc.
     */

    /* MQTT(S) operation result codes */
    typedef enum
    {
        SIM76xx_MQTT_OK = 0,                                     /**< Operation succeeded */
        SIM76xx_MQTT_FAILED = 1,                                 /**< General failure */
        SIM76xx_MQTT_BAD_UTF8 = 2,                               /**< Bad UTF-8 string */
        SIM76xx_MQTT_SOCK_CONNECT_FAIL = 3,                      /**< Socket connect failed */
        SIM76xx_MQTT_SOCK_CREATE_FAIL = 4,                       /**< Socket creation failed */
        SIM76xx_MQTT_SOCK_CLOSE_FAIL = 5,                        /**< Socket close failed */
        SIM76xx_MQTT_MSG_RECV_FAIL = 6,                          /**< Message receive failed */
        SIM76xx_MQTT_NET_OPEN_FAIL = 7,                          /**< Network open failed */
        SIM76xx_MQTT_NET_CLOSE_FAIL = 8,                         /**< Network close failed */
        SIM76xx_MQTT_NET_NOT_OPEN = 9,                           /**< Network not opened */
        SIM76xx_MQTT_CLIENT_INDEX_ERROR = 10,                    /**< Client index error */
        SIM76xx_MQTT_NO_CONNECTION = 11,                         /**< No connection */
        SIM76xx_MQTT_INVALID_PARAM = 12,                         /**< Invalid parameter */
        SIM76xx_MQTT_NOT_SUPPORTED = 13,                         /**< Operation not supported */
        SIM76xx_MQTT_CLIENT_BUSY = 14,                           /**< Client is busy */
        SIM76xx_MQTT_REQUIRE_CONN_FAIL = 15,                     /**< Required connection failed */
        SIM76xx_MQTT_SOCK_SEND_FAIL = 16,                        /**< Socket sending failed */
        SIM76xx_MQTT_TIMEOUT = 17,                               /**< Timeout */
        SIM76xx_MQTT_TOPIC_EMPTY = 18,                           /**< Topic is empty */
        SIM76xx_MQTT_CLIENT_USED = 19,                           /**< Client already used */
        SIM76xx_MQTT_CLIENT_NOT_ACQUIRED = 20,                   /**< Client not acquired */
        SIM76xx_MQTT_CLIENT_NOT_RELEASED = 21,                   /**< Client not released */
        SIM76xx_MQTT_LENGTH_OUT_OF_RANGE = 22,                   /**< Length out of range */
        SIM76xx_MQTT_NET_ALREADY_OPEN = 23,                      /**< Network already opened */
        SIM76xx_MQTT_PACKET_FAIL = 24,                           /**< Packet failure */
        SIM76xx_MQTT_DNS_ERROR = 25,                             /**< DNS error */
        SIM76xx_MQTT_SOCKET_CLOSED_BY_SERVER = 26,               /**< Socket closed by server */
        SIM76xx_MQTT_CONN_REFUSED_BAD_PROTO = 27,                /**< Connection refused: unaccepted protocol version */
        SIM76xx_MQTT_CONN_REFUSED_ID_REJECTED = 28,              /**< Connection refused: identifier rejected */
        SIM76xx_MQTT_CONN_REFUSED_SERVER_UNAVAILABLE = 29,       /**< Connection refused: server unavailable */
        SIM76xx_MQTT_CONN_REFUSED_BAD_USERNAME_OR_PASSWORD = 30, /**< Connection refused: bad user name or password */
        SIM76xx_MQTT_CONN_REFUSED_NOT_AUTHORIZED = 31,           /**< Connection refused: not authorized */
        SIM76xx_MQTT_HANDSHAKE_FAILED = 32,                      /**< handshake fail */
        SIM76xx_MQTT_NOT_SET_CERTIFICATE = 33,                   /**< not set certificate */
        SIM76xx_MQTT_OPEN_SSL_SESSION_FAILED = 34                /**< open SSL session failed */

    } SIM76xx_mqtt_err_t;

    /**
     * @brief Convert MQTT error code to human-readable string
     * @param err MQTT error code
     * @return const char* description
     */
    static inline const char *SIM76xx_mqtt_err_str(SIM76xx_mqtt_err_t err)
    {
        switch (err)
        {
        case SIM76xx_MQTT_OK:
            return "Operation succeeded";
        case SIM76xx_MQTT_FAILED:
            return "General failure";
        case SIM76xx_MQTT_BAD_UTF8:
            return "Bad UTF-8 string";
        case SIM76xx_MQTT_SOCK_CONNECT_FAIL:
            return "Socket connect failed";
        case SIM76xx_MQTT_SOCK_CREATE_FAIL:
            return "Socket create failed";
        case SIM76xx_MQTT_SOCK_CLOSE_FAIL:
            return "Socket close failed";
        case SIM76xx_MQTT_MSG_RECV_FAIL:
            return "Message receive failed";
        case SIM76xx_MQTT_NET_OPEN_FAIL:
            return "Network open failed";
        case SIM76xx_MQTT_NET_CLOSE_FAIL:
            return "Network close failed";
        case SIM76xx_MQTT_NET_NOT_OPEN:
            return "Network not opened";
        case SIM76xx_MQTT_CLIENT_INDEX_ERROR:
            return "Client index error";
        case SIM76xx_MQTT_NO_CONNECTION:
            return "No connection";
        case SIM76xx_MQTT_INVALID_PARAM:
            return "Invalid parameter";
        case SIM76xx_MQTT_NOT_SUPPORTED:
            return "Operation not supported";
        case SIM76xx_MQTT_CLIENT_BUSY:
            return "Client busy";
        case SIM76xx_MQTT_REQUIRE_CONN_FAIL:
            return "Required connection failed";
        case SIM76xx_MQTT_SOCK_SEND_FAIL:
            return "Socket send failed";
        case SIM76xx_MQTT_TIMEOUT:
            return "Timeout";
        case SIM76xx_MQTT_TOPIC_EMPTY:
            return "Topic is empty";
        case SIM76xx_MQTT_CLIENT_USED:
            return "Client already used";
        case SIM76xx_MQTT_CLIENT_NOT_ACQUIRED:
            return "Client not acquired";
        case SIM76xx_MQTT_CLIENT_NOT_RELEASED:
            return "Client not released";
        case SIM76xx_MQTT_LENGTH_OUT_OF_RANGE:
            return "Length out of range";
        case SIM76xx_MQTT_NET_ALREADY_OPEN:
            return "Network already opened";
        case SIM76xx_MQTT_PACKET_FAIL:
            return "Packet failure";
        case SIM76xx_MQTT_DNS_ERROR:
            return "DNS error";
        case SIM76xx_MQTT_SOCKET_CLOSED_BY_SERVER:
            return "Socket closed by server";
        case SIM76xx_MQTT_CONN_REFUSED_BAD_PROTO:
            return "Connection refused: bad protocol version";
        case SIM76xx_MQTT_CONN_REFUSED_ID_REJECTED:
            return "Connection refused: identifier rejected";
        case SIM76xx_MQTT_CONN_REFUSED_SERVER_UNAVAILABLE:
            return "Connection refused: server unavailable";
        case SIM76xx_MQTT_CONN_REFUSED_BAD_USERNAME_OR_PASSWORD:
            return "Connection refused: bad user name or password";
        case SIM76xx_MQTT_CONN_REFUSED_NOT_AUTHORIZED:
            return "Connection refused: not authorized";
        case SIM76xx_MQTT_HANDSHAKE_FAILED:
            return "Handshake fail";
        case SIM76xx_MQTT_NOT_SET_CERTIFICATE:
            return "Not set certificate";
        case SIM76xx_MQTT_OPEN_SSL_SESSION_FAILED:
            return "Open SSL session failed";
        default:
            return "Unknown MQTT error";
        }
    }

#ifdef __cplusplus
}
#endif

#endif /* SIM76xx_MQTT_ERRORS_H */
