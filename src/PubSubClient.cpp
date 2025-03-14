/*
  PubSubClient.cpp - A simple client for MQTT.
  Nick O'Leary, Holger Mueller
  http://knolleary.net
  https://github.com/hmueller01/pubsubclient3
*/

#include "PubSubClient.h"

PubSubClient::PubSubClient()
    : _client(nullptr),
      bufferSize(0),
      keepAlive(MQTT_KEEPALIVE),
      socketTimeout(MQTT_SOCKET_TIMEOUT),
      callback(nullptr),
      domain(nullptr),
      port(0),
      stream(nullptr),
      _state(MQTT_DISCONNECTED) {
    setBufferSize(MQTT_MAX_PACKET_SIZE);
}

PubSubClient::PubSubClient(Client& client) : PubSubClient() {
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, Client& client) : PubSubClient() {
    setServer(addr, port);
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(addr, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(addr, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(addr, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, Client& client) : PubSubClient() {
    setServer(ip, port);
    setClient(client);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(ip, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(ip, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(ip, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, Client& client) : PubSubClient() {
    setServer(domain, port);
    setClient(client);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(domain, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(domain, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(domain, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::~PubSubClient() {
    free(this->domain);
    free(this->buffer);
}

bool PubSubClient::connect(const char* id) {
    return connect(id, NULL, NULL, 0, 0, 0, 0, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass) {
    return connect(id, user, pass, 0, 0, 0, 0, 1);
}

bool PubSubClient::connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
    return connect(id, NULL, NULL, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain,
                           const char* willMessage) {
    return connect(id, user, pass, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain,
                           const char* willMessage, bool cleanSession) {
    if (!connected()) {
        int result = 0;

        if (_client->connected()) {
            result = 1;
        } else if (this->port != 0) {
            if (this->domain != NULL) {
                result = _client->connect(this->domain, this->port);
            } else {
                result = _client->connect(this->ip, this->port);
            }
        }

        if (result == 1) {
            nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            size_t length = MQTT_MAX_HEADER_SIZE;

#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = {0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = {0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (size_t j = 0; j < MQTT_HEADER_VERSION_LENGTH; j++) {
                this->buffer[length++] = d[j];
            }

            uint8_t v;
            if (willTopic) {
                v = 0x04 | (willQos << 3) | (willRetain << 5);
            } else {
                v = 0x00;
            }
            if (cleanSession) {
                v = v | 0x02;
            }

            if (user != NULL) {
                v = v | 0x80;

                if (pass != NULL) {
                    v = v | (0x80 >> 1);
                }
            }
            this->buffer[length++] = v;
            this->buffer[length++] = ((this->keepAlive) >> 8);
            this->buffer[length++] = ((this->keepAlive) & 0xFF);

            CHECK_STRING_LENGTH(length, id)
            length = writeString(id, this->buffer, length);
            if (willTopic) {
                CHECK_STRING_LENGTH(length, willTopic)
                length = writeString(willTopic, this->buffer, length);
                CHECK_STRING_LENGTH(length, willMessage)
                length = writeString(willMessage, this->buffer, length);
            }

            if (user != NULL) {
                CHECK_STRING_LENGTH(length, user)
                length = writeString(user, this->buffer, length);
                if (pass != NULL) {
                    CHECK_STRING_LENGTH(length, pass)
                    length = writeString(pass, this->buffer, length);
                }
            }

            write(MQTTCONNECT, this->buffer, length - MQTT_MAX_HEADER_SIZE);

            lastInActivity = lastOutActivity = millis();
            pingOutstanding = false;

            while (!_client->available()) {
                yield();
                unsigned long t = millis();
                if (t - lastInActivity >= this->socketTimeout * 1000UL) {
                    DEBUG_PSC_PRINTF("connect aborting due to timeout\n");
                    _state = MQTT_CONNECTION_TIMEOUT;
                    _client->stop();
                    return false;
                }
            }
            uint8_t llen;
            size_t len = readPacket(&llen);

            if (len == 4) {
                if (buffer[3] == 0) {
                    lastInActivity = millis();
                    _state = MQTT_CONNECTED;
                    return true;
                } else {
                    _state = buffer[3];
                }
            }
            DEBUG_PSC_PRINTF("connect aborting due to protocol error\n");
            _client->stop();
        } else {
            _state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}

/**
 * @brief  Reads a byte into result.
 *
 * @param  result Pointer to result buffer.
 * @return true if byte was read, false if socketTimeout occurred.
 */
bool PubSubClient::readByte(uint8_t* result) {
    unsigned long previousMillis = millis();
    while (!_client->available()) {
        yield();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= this->socketTimeout * 1000UL) {
            return false;
        }
    }
    *result = _client->read();
    return true;
}

/**
 * @brief  Reads a byte into result[*index] and increments *index.
 * Note: *index may go out of bounds of result. This must be checked outside of this function!
 *
 * @return true if a byte was read, otherwise false (socketTimeout).
 */
bool PubSubClient::readByte(uint8_t* result, size_t* index) {
    uint8_t* write_address = &(result[*index]);
    if (readByte(write_address)) {
        (*index)++;
        return true;
    }
    return false;
}

/**
 * @brief  Reads a complete packet (header, topic, payload) into this->buffer.
 *
 * @param  *lenLen Returns the variable header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1)
 * @return Number of read bytes, 0 in case of an error (socketTimeout, buffer overflow)
 */
size_t PubSubClient::readPacket(uint8_t* lenLen) {
    size_t len = 0;
    if (!readByte(this->buffer, &len)) return 0;
    bool isPublish = (this->buffer[0] & 0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    size_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint8_t start = 0;

    do {
        if (len == MQTT_MAX_HEADER_SIZE) {
            // Invalid remaining length encoding - kill the connection
            _state = MQTT_DISCONNECTED;
            DEBUG_PSC_PRINTF("readPacket detected packet of invalid length\n");
            _client->stop();
            return 0;
        }
        if (!readByte(&digit)) return 0;
        this->buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier <<= 7;  // multiplier *= 128
    } while ((digit & 128) != 0);
    *lenLen = (uint8_t)(len - 1);

    DEBUG_PSC_PRINTF("readPacket received packet of length %zu (isPublish = %u)\n", length, isPublish);

    if (isPublish) {
        // Read in topic length to calculate bytes to skip over for Stream writing
        if (!readByte(this->buffer, &len)) return 0;
        if (!readByte(this->buffer, &len)) return 0;
        skip = (this->buffer[*lenLen + 1] << 8) + this->buffer[*lenLen + 2];
        start = 2;
        if (this->buffer[0] & MQTTQOS1) {
            // skip message id
            skip += 2;
        }
    }
    size_t idx = len;

    for (size_t i = start; i < length; i++) {
        if (!readByte(&digit)) return 0;
        if (this->stream) {
            if (isPublish && idx - *lenLen - 2 > skip) {
                this->stream->write(digit);
            }
        }

        if (len < this->bufferSize) {
            this->buffer[len++] = digit;
        }
        idx++;
    }

    if (!this->stream && idx > this->bufferSize) {
        DEBUG_PSC_PRINTF("readPacket ignoring packet of size %zu exceeding buffer of size %zu\n", length, this->bufferSize);
        len = 0;  // This will cause the packet to be ignored.
    }
    return len;
}

/**
 * @brief  After a packet is read handle the content here (call the callback, handle pings)
 *
 * @param  llen Header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1)
 * @param  len Number of read bytes in this->buffer
 */
void PubSubClient::handlePacket(uint8_t llen, size_t len) {
    uint8_t type = this->buffer[0] & 0xF0;
    DEBUG_PSC_PRINTF("received message of type %u\n", type);
    switch (type) {
        case MQTTPUBLISH:
            if (callback) {
                uint8_t* payload;
                uint16_t tl = (this->buffer[llen + 1] << 8) + this->buffer[llen + 2]; /* topic length in bytes */
                memmove(this->buffer + llen + 2, this->buffer + llen + 3, tl);        /* move topic inside buffer 1 byte to front */
                this->buffer[llen + 2 + tl] = 0;                                      /* end the topic as a 'C' string with \x00 */
                char* topic = (char*)this->buffer + llen + 2;
                // msgId only present for QOS>0
                if ((this->buffer[0] & 0x06) == MQTTQOS1) {
                    uint16_t msgId = (this->buffer[llen + 3 + tl] << 8) + this->buffer[llen + 3 + tl + 1];
                    payload = this->buffer + llen + 3 + tl + 2;
                    callback(topic, payload, len - llen - 3 - tl - 2);

                    this->buffer[0] = MQTTPUBACK;
                    this->buffer[1] = 2;
                    this->buffer[2] = (msgId >> 8);
                    this->buffer[3] = (msgId & 0xFF);
                    if (_client->write(this->buffer, 4) == 4) {
                        lastOutActivity = millis();
                    }
                } else {
                    // No msgId
                    payload = this->buffer + llen + 3 + tl;
                    callback(topic, payload, len - llen - 3 - tl);
                }
            }
            break;
        case MQTTPINGREQ:
            this->buffer[0] = MQTTPINGRESP;
            this->buffer[1] = 0;
            if (_client->write(this->buffer, 2) == 2) {
                lastOutActivity = millis();
            }
            break;
        case MQTTPINGRESP:
            pingOutstanding = false;
            break;
        default:
            break;
    }
}

bool PubSubClient::loop() {
    if (!connected()) {
        return false;
    }
    unsigned long t = millis();
    if (((t - lastInActivity > this->keepAlive * 1000UL) || (t - lastOutActivity > this->keepAlive * 1000UL)) && keepAlive != 0) {
        if (pingOutstanding) {
            this->_state = MQTT_CONNECTION_TIMEOUT;
            DEBUG_PSC_PRINTF("loop aborting due to timeout\n");
            _client->stop();
            pingOutstanding = false;
            return false;
        } else {
            this->buffer[0] = MQTTPINGREQ;
            this->buffer[1] = 0;
            if (_client->write(this->buffer, 2) == 2) {
                lastOutActivity = t;
                lastInActivity = t;
                pingOutstanding = true;
            }
        }
    }
    if (_client->available()) {
        uint8_t lenLen;
        size_t len = readPacket(&lenLen);
        if (len > 0) {
            lastInActivity = t;
            handlePacket(lenLen, len);
        } else if (!connected()) {
            // readPacket has closed the connection
            return false;
        }
    }
    return true;
}

bool PubSubClient::publish(const char* topic, const char* payload) {
    return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, this->bufferSize) : 0, false);
}

bool PubSubClient::publish(const char* topic, const char* payload, bool retained) {
    return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, this->bufferSize) : 0, retained);
}

bool PubSubClient::publish(const char* topic, const uint8_t* payload, size_t plength) {
    return publish(topic, payload, plength, false);
}

bool PubSubClient::publish(const char* topic, const uint8_t* payload, size_t plength, bool retained) {
    if (connected()) {
        if (this->bufferSize < MQTT_MAX_HEADER_SIZE + 2 + strnlen(topic, this->bufferSize) + plength) {
            // Too long
            return false;
        }
        // Leave room in the buffer for header and variable length field
        size_t length = MQTT_MAX_HEADER_SIZE;
        length = writeString(topic, this->buffer, length);

        // Add payload
        for (size_t i = 0; i < plength; i++) {
            this->buffer[length++] = payload[i];
        }

        // Write the header
        const uint8_t header = MQTTPUBLISH | (retained ? MQTTRETAINED : 0);
        return write(header, this->buffer, length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

bool PubSubClient::publish_P(const char* topic, const char* payload, bool retained) {
    return publish_P(topic, (const uint8_t*)payload, payload ? strnlen_P(payload, MQTT_MAX_POSSIBLE_PACKET_SIZE) : 0, retained);
}

bool PubSubClient::publish_P(const char* topic, const uint8_t* payload, size_t plength, bool retained) {
    uint8_t llen = 0;
    uint8_t digit;
    size_t rc = 0;
    size_t tlen;
    size_t pos = 0;
    const uint8_t header = MQTTPUBLISH | (retained ? MQTTRETAINED : 0);
    size_t len;
    size_t expectedLength;

    if (!connected()) {
        return false;
    }

    tlen = strnlen(topic, this->bufferSize);

    this->buffer[pos++] = header;
    len = plength + 2 + tlen;
    do {
        digit = len & 127;  // digit = len % 128
        len >>= 7;          // len = len / 128
        if (len > 0) {
            digit |= 0x80;
        }
        this->buffer[pos++] = digit;
        llen++;
    } while (len > 0);

    pos = writeString(topic, this->buffer, pos);

    rc += _client->write(this->buffer, pos);

    for (size_t i = 0; i < plength; i++) {
        rc += _client->write((uint8_t)pgm_read_byte_near(payload + i));
    }

    lastOutActivity = millis();

    expectedLength = 1 + llen + 2 + tlen + plength;

    return (rc == expectedLength);
}

bool PubSubClient::beginPublish(const char* topic, size_t plength, bool retained) {
    if (connected()) {
        // Send the header and variable length field
        size_t length = MQTT_MAX_HEADER_SIZE;
        length = writeString(topic, this->buffer, length);
        const uint8_t header = MQTTPUBLISH | (retained ? MQTTRETAINED : 0);
        uint8_t hlen = buildHeader(header, this->buffer, plength + length - MQTT_MAX_HEADER_SIZE);
        size_t rc = _client->write(this->buffer + (MQTT_MAX_HEADER_SIZE - hlen), length - (MQTT_MAX_HEADER_SIZE - hlen));
        lastOutActivity = millis();
        return (rc == (length - (MQTT_MAX_HEADER_SIZE - hlen)));
    }
    return false;
}

int PubSubClient::endPublish() {
    return 1;
}

size_t PubSubClient::write(uint8_t data) {
    lastOutActivity = millis();
    return _client->write(data);
}

size_t PubSubClient::write(const uint8_t* buffer, size_t size) {
    lastOutActivity = millis();
    return _client->write(buffer, size);
}

/**
 * @brief  Build up the header ready to send.
 * Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
 * (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the buffer.
 *
 * @param  header Header byte, e.g. MQTTCONNECT, MQTTPUBLISH, MQTTSUBSCRIBE, MQTTUNSUBSCRIBE.
 * @param  buf Buffer to write header to.
 * @param  length Length to encode in the header.
 * @return Returns the size of the header (1 .. MQTT_MAX_HEADER_SIZE).
 */
uint8_t PubSubClient::buildHeader(uint8_t header, uint8_t* buf, size_t length) {
    uint8_t lenBuf[MQTT_MAX_HEADER_SIZE - 1];
    uint8_t lenLen = 0;
    uint8_t digit;
    size_t len = length;
    do {
        digit = len & 127;  // digit = len % 128
        len >>= 7;          // len = len / 128
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[lenLen++] = digit;
    } while (len > 0 && lenLen < MQTT_MAX_HEADER_SIZE - 1);

    if (len > 0) {
        DEBUG_PSC_PRINTF("length too big %zu, left %zu, should be 0\r\n", length, len);
    }

    buf[MQTT_MAX_HEADER_SIZE - 1 - lenLen] = header;
    for (uint8_t i = 0; i < lenLen; i++) {
        buf[MQTT_MAX_HEADER_SIZE - lenLen + i] = lenBuf[i];
    }
    return lenLen + 1;  // Full header size is variable length bit plus the 1-byte fixed header
}

bool PubSubClient::write(uint8_t header, uint8_t* buf, size_t length) {
    bool result = true;
    size_t rc;
    uint8_t hlen = buildHeader(header, buf, length);

#ifdef MQTT_MAX_TRANSFER_SIZE
    uint8_t* writeBuf = buf + (MQTT_MAX_HEADER_SIZE - hlen);
    size_t bytesRemaining = length + hlen;  // Match the length type
    size_t bytesToWrite;
    while ((bytesRemaining > 0) && result) {
        yield();
        bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
        rc = _client->write(writeBuf, bytesToWrite);
        result = (rc == bytesToWrite);
        bytesRemaining -= rc;
        writeBuf += rc;
        if (result) {
            lastOutActivity = millis();
        }
    }
#else
    rc = _client->write(buf + (MQTT_MAX_HEADER_SIZE - hlen), length + hlen);
    result = (rc == length + hlen);
    if (result) {
        lastOutActivity = millis();
    }
#endif
    return result;
}

bool PubSubClient::subscribe(const char* topic) {
    return subscribe(topic, 0);
}

bool PubSubClient::subscribe(const char* topic, uint8_t qos) {
    size_t topicLength = strnlen(topic, this->bufferSize);
    if (topic == 0) {
        return false;
    }
    if (qos > 1) {
        return false;
    }
    if (this->bufferSize < 9 + topicLength) {
        // Too long
        return false;
    }
    if (connected()) {
        // Leave room in the buffer for header and variable length field
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        this->buffer[length++] = (nextMsgId >> 8);
        this->buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, this->buffer, length);
        this->buffer[length++] = qos;
        return write(MQTTSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

bool PubSubClient::unsubscribe(const char* topic) {
    size_t topicLength = strnlen(topic, this->bufferSize);
    if (topic == 0) {
        return false;
    }
    if (this->bufferSize < 9 + topicLength) {
        // Too long
        return false;
    }
    if (connected()) {
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        this->buffer[length++] = (nextMsgId >> 8);
        this->buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, this->buffer, length);
        return write(MQTTUNSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

void PubSubClient::disconnect() {
    this->buffer[0] = MQTTDISCONNECT;
    this->buffer[1] = 0;
    _client->write(this->buffer, 2);
    _state = MQTT_DISCONNECTED;
    _client->flush();
    DEBUG_PSC_PRINTF("disconnect called\n");
    _client->stop();
    lastInActivity = lastOutActivity = millis();
    pingOutstanding = false;
}

size_t PubSubClient::writeString(const char* string, uint8_t* buf, size_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = (uint8_t)*idp++;
        i++;
    }
    buf[pos - i - 2] = (i >> 8);
    buf[pos - i - 1] = (i & 0xFF);
    return pos;
}

bool PubSubClient::connected() {
    bool rc;
    if (_client == NULL) {
        rc = false;
    } else {
        rc = (bool)_client->connected();
        if (!rc) {
            if (this->_state == MQTT_CONNECTED) {
                this->_state = MQTT_CONNECTION_LOST;
                DEBUG_PSC_PRINTF("lost connection (client may have more details)\n");
                _client->flush();
                _client->stop();
            }
        } else {
            return this->_state == MQTT_CONNECTED;
        }
    }
    return rc;
}

PubSubClient& PubSubClient::setServer(uint8_t* ip, uint16_t port) {
    IPAddress addr(ip[0], ip[1], ip[2], ip[3]);
    return setServer(addr, port);
}

PubSubClient& PubSubClient::setServer(IPAddress ip, uint16_t port) {
    this->ip = ip;
    this->port = port;
    free(this->domain);
    this->domain = NULL;
    return *this;
}

PubSubClient& PubSubClient::setServer(const char* domain, uint16_t port) {
    char* newDomain = NULL;
    if (domain != NULL) {
        newDomain = (char*)realloc(this->domain, strlen(domain) + 1);
    }
    if (newDomain != NULL) {
        strcpy(newDomain, domain);
        this->domain = newDomain;
        this->port = port;
    } else {
        free(this->domain);
        this->domain = NULL;
        this->port = 0;
    }
    return *this;
}

PubSubClient& PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

PubSubClient& PubSubClient::setClient(Client& client) {
    this->_client = &client;
    return *this;
}

PubSubClient& PubSubClient::setStream(Stream& stream) {
    this->stream = &stream;
    return *this;
}

int PubSubClient::state() {
    return this->_state;
}

bool PubSubClient::setBufferSize(size_t size) {
    if (size == 0) {
        // Cannot set it back to 0
        return false;
    }
    if (this->bufferSize == 0) {
        this->buffer = (uint8_t*)malloc(size);
    } else {
        uint8_t* newBuffer = (uint8_t*)realloc(this->buffer, size);
        if (newBuffer != NULL) {
            this->buffer = newBuffer;
        } else {
            return false;
        }
    }
    this->bufferSize = size;
    return (this->buffer != NULL);
}

size_t PubSubClient::getBufferSize() {
    return this->bufferSize;
}

PubSubClient& PubSubClient::setKeepAlive(uint16_t keepAlive) {
    this->keepAlive = keepAlive;
    return *this;
}

PubSubClient& PubSubClient::setSocketTimeout(uint16_t timeout) {
    this->socketTimeout = timeout;
    return *this;
}
