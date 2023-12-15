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
from enum import Enum
import queue

BROKER_ADDR = "broker.hivemq.com"
PORT = 1883
PUB_TOPIC = "foem/db_py_intf/v1_0_0/v2o"
SUB_TOPIC = "foem/db_py_intf/v1_0_0/o2v"
CLIENT_ID = "fvehicle1"

USERNAME = "mashraf"
PWD = "Gttllee1332"


class SimpleMQTTClient:
    class FotaCommands(Enum):
        UPDATE_REQUEST = "1332"
        SEND_FIRMWARE_DONE = "50"
        SEND_FIRMWARE_HASH_DONE = "51"

    class fotaOemResponse(Enum):
        OEM_ACK = "40"
        OEM_NACK = "41"

    class FotaVehicleResponse(Enum):
        ACK = "69"
        NACK = "70"
        SEND_FIRMWARE = "30"
        SEND_FIRMWARE_HASH = "31"
        DONE = "99"

    class FotaErrorCode(Enum):
        INVALID_CMD = "0"
        BAD_RESPONE = "1"

    def __init__(self, broker_address, port, client_id):
        self.broker_address = broker_address
        self.port = port
        self.client_id = client_id
        self.client = mqtt.Client(client_id)

        # Set callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.client.username_pw_set(username=USERNAME, password=PWD)

        self.__fota_available_commands = {
            "UPDATE_REQUEST": "Update is ready to be sent",
        }
        self.__fota_commands = {
            SimpleMQTTClient.FotaCommands.UPDATE_REQUEST.value: self.__fota_cmd_handle_UPDATE_REQUEST,
        }

        self.__mqtt_msgsQ = queue.Queue()
        self.__expectedCmds = [
            SimpleMQTTClient.FotaCommands.UPDATE_REQUEST.value,
            SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_DONE.value,
            SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_HASH_DONE.value,
        ]

    def on_connect(self, client, userdata, flags, rc):
        if not rc == 0:
            print(f"Error with result code {rc}")

    def on_message(self, client, userdata, msg):
        # Extract the publisher's Client ID from the topic
        self.__mqtt_currmsg = msg.payload.decode("UTF-8")
        print(
            f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
        )
        self.__mqtt_msgsQ.put(str(self.__mqtt_currmsg))

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

    def __sendToOEM(self, msg):
        self.publish(PUB_TOPIC, msg)
        print(f"published: `{msg}` on topic `{PUB_TOPIC}`")

    def __sendAck(self):
        self.__sendToOEM(f"{SimpleMQTTClient.FotaVehicleResponse.ACK.value}")

    def __sendNack(self, ErrorCode):
        self.__sendToOEM(
            f"{SimpleMQTTClient.FotaVehicleResponse.ACK.value}, {ErrorCode}"
        )

    def __fota_cmd_exec(self, command):
        default_command = lambda: 0
        command_method = self.__fota_commands.get(command, default_command)
        return command_method()

    def __fota_cmd_handle_UPDATE_REQUEST(self):
        firmwareRx = []
        firmwareHashRx = ""

        self.__sendAck()

        self.__sendToOEM(SimpleMQTTClient.FotaVehicleResponse.SEND_FIRMWARE.value)
        if self.__mqtt_msgsQ.get() == SimpleMQTTClient.fotaOemResponse.OEM_ACK.value:
            pass

        # Start listening for the firmware hex line by line
        while True:
            firmware_msg = self.__mqtt_msgsQ.get()
            if firmware_msg == SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_DONE.value:
                self.__sendAck()
                break
            else:
                firmwareRx.append(str(firmware_msg))

        # Start listening for the firmware hash
        self.__sendToOEM(SimpleMQTTClient.FotaVehicleResponse.SEND_FIRMWARE_HASH.value)
        if self.__mqtt_msgsQ.get() == SimpleMQTTClient.fotaOemResponse.OEM_ACK.value:
            pass
        
        while True:
            firmware_hash_msg = self.__mqtt_msgsQ.get()
            if firmware_hash_msg == SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_HASH_DONE.value:
                self.__sendAck()
                break
            else:
                firmwareHashRx = str(firmware_hash_msg)

        # Save data
        self.__save_firmware(firmwareRx)
        self.__save_firmwareHash(firmwareHashRx)
        # End Of Comm
        self.__sendToOEM(SimpleMQTTClient.FotaVehicleResponse.DONE.value)

    def __save_firmware(self, firmware):
        with open("UpdatedFirmware.hex", "wb") as f:
            for line in firmware:
                f.write(line.encode('utf-8') + b'\n')

    def __save_firmwareHash(self, firmwareHash):
        with open("UpdatedFirmwareHash.txt", "w") as f:
            f.write(firmwareHash)

    def __vehicle_cmd_handle(self):
        if not self.__mqtt_msgsQ.empty():
            oem_msg = self.__mqtt_msgsQ.get()
            return self.__fota_cmd_exec(oem_msg)

    def run(self):
        vehicle_btl = btl_ttl_intf(True, False)

        # Setup bootloader cfg
        # vehicle_btl.set_serial_port("/dev/ttyUSB0")
        # vehicle_btl.set_baudrate(115200)
        # vehicle_btl.set_appBinFp("./TestAppLedOn.bin")

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
                    self.__vehicle_cmd_handle()
                except KeyboardInterrupt:
                    break
        finally:
            mqtt_client.loop_stop()
            mqtt_client.disconnect()


# Example usage:
if __name__ == "__main__":
    mqtt_client = SimpleMQTTClient(BROKER_ADDR, PORT, CLIENT_ID)
    mqtt_client.run()
