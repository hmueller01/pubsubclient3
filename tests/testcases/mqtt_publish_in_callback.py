import unittest
import settings
import time
import paho.mqtt.client as mqttclinet


class mqtt_publish_in_callback(unittest.TestCase):
    def on_message(self, client, userdata, message: mqttclinet.MQTTMessage):
        self.message_queue.append(message)

    message_queue = []

    @classmethod
    def setUpClass(self):
        self.client = mqttclinet.Client("pubsubclient_ut", clean_session=True)
        self.client.connect(settings.server_ip)
        self.client.on_message = self.on_message
        self.client.subscribe("outTopic", 0)

    @classmethod
    def tearDownClass(self):
        self.client.disconnect()

    def test_connect(self):
        i = 30
        while len(self.message_queue) == 0 and i > 0:
            self.client.loop()
            time.sleep(0.5)
            i -= 1
        self.assertTrue(i > 0, "message receive timed-out")
        self.assertEqual(len(self.message_queue), 1, "unexpected number of messages received")
        msg = self.message_queue.pop(0)
        self.assertEqual(msg.mid, 0, "message id not 0")
        self.assertEqual(msg.topic, "outTopic", "message topic incorrect")
        self.assertEqual(msg.payload, "hello world")
        self.assertEqual(msg.qos, 0, "message qos not 0")
        self.assertEqual(msg.retain, False, "message retain flag incorrect")

    def test_publish(self):
        self.assertEqual(len(self.message_queue), 0, "message queue not empty")
        payload = "abcdefghij"
        self.client.publish("inTopic", payload)

        i = 30
        while len(self.message_queue) == 0 and i > 0:
            self.client.loop()
            time.sleep(0.5)
            i -= 1

        self.assertTrue(i > 0, "message receive timed-out")
        self.assertEqual(len(self.message_queue), 1, "unexpected number of messages received")
        msg = self.message_queue.pop(0)
        self.assertEqual(msg.mid, 0, "message id not 0")
        self.assertEqual(msg.topic, "outTopic", "message topic incorrect")
        self.assertEqual(msg.payload, payload)
        self.assertEqual(msg.qos, 0, "message qos not 0")
        self.assertEqual(msg.retain, False, "message retain flag incorrect")
