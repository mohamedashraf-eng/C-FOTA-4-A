"""
  @author Mohamed Ashraf Wx
  @date 2023 / 13 / 3
  @brief
  @copyright Copyright (c) Mohamed Ashraf, 2023 FOTA

  @attention
"""
import time
from btl_intf import btl_ttl_intf
import paho.mqtt.client as mqtt

BROKER_ADDR = "broker.hivemq.com"
PORT = 1883
PUB_TOPIC = "foem/db_py_intf/v1_0_0/v2o"
SUB_TOPIC = "foem/db_py_intf/v1_0_0/o2v"
CLIENT_ID = "fvehicle1"

USERNAME = "mashraf"
PWD = "Gttllee1332"

class SimpleMQTTClient:
	def __init__(self, broker_address, port, client_id):
		self.broker_address = broker_address
		self.port = port
		self.client_id = client_id
		self.client = mqtt.Client(client_id)

		# Set callbacks
		self.client.on_connect = self.on_connect
		self.client.on_message = self.on_message
  
		self.client.username_pw_set(
		username=USERNAME, password=PWD
		)
  
	def on_connect(self, client, userdata, flags, rc):
		if not rc == 0:
			print(f"Error with result code {rc}")

	def on_message(self, client, userdata, msg):
		# Extract the publisher's Client ID from the topic
		mqtt_currmsg = msg.payload.decode("UTF-8")
		print(
			f"\nReceived message from topic: `{msg.topic}` msg: `{mqtt_currmsg}`"
		)

	def connect(self):
		self.client.connect(self.broker_address, self.port, 60)

	def disconnect(self):
		self.client.disconnect(self.broker_address)
  
	def publish(self, topic, message):
		self.client.publish(topic, message)

	def subscribe(self, topic):
		self.client.subscribe(topic)

	def loop_start(self):
		self.client.loop_start()
  
	def loop_stop(self):
		self.client.loop_stop()

# Example usage:
if __name__ == "__main__":
	mqtt_client = SimpleMQTTClient(BROKER_ADDR, PORT, CLIENT_ID)
	vehicle_btl = btl_ttl_intf(True, False)
	
	# Setup bootloader cfg
	vehicle_btl.set_serial_port("/dev/ttyUSB0")
	vehicle_btl.set_baudrate(115200)
	vehicle_btl.set_appBinFp("./TestAppLedOn.bin")
 
	# vehicle_btl.btl_command_exec(vehicle_btl.BtlCommands.CBL_FLASH_ERASE_CMD.value)
 
	# Setup mqtt cfg
	mqtt_client.connect()
	time.sleep(1)  # Add a short delay after connecting
	mqtt_client.subscribe(SUB_TOPIC)
	mqtt_client.loop_start()

	# Start the channel
	try:
		while True:
			try:
				msg_to_publish = str(input("Enter msg to publish from vehicle: "))
				mqtt_client.publish(PUB_TOPIC, msg_to_publish)
			except KeyboardInterrupt:
				break
	finally:
		mqtt_client.loop_stop()
		mqtt_client.disconnect()





