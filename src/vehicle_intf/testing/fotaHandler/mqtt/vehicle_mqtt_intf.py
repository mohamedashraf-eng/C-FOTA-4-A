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
import hashlib
import time
import random
import ssl
import gzip

BROKER_ADDR = "192.168.1.4"
PORT = 8883
PUB_TOPIC = "foem/db_py_intf/v1_0_0/v2o"
SUB_TOPIC = "foem/db_py_intf/v1_0_0/o2v"
CLIENT_ID = "fvehicle1"

# SSL/TLS
CLIENT_CERT = r".\mqtt_certs\client\client1.crt"
CLIENT_KEY = r".\mqtt_certs\client\client1.key"
CA_CERT = r".\mqtt_certs\ca\ca.crt"
KEY_PASSPHRASE = "1332"


def set_ssl():
    ssl_context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    ssl_context.verify_mode = ssl.CERT_REQUIRED
    ssl_context.load_cert_chain(
        certfile=CLIENT_CERT, keyfile=CLIENT_KEY, password=KEY_PASSPHRASE
    )
    ssl_context.load_verify_locations(cafile=CA_CERT)
    return ssl_context


def update_credentials():
    # Use current time and random salt to update credentials
    current_time = str(int(time.time()))
    random_salt = str(random.randint(1, 10000))

    # Concatenate time, salt, and any additional information if needed
    data_to_hash = current_time + random_salt + "additional_info"

    # Hash the concatenated data using SHA-512
    hashed_data = hashlib.sha512(data_to_hash.encode()).hexdigest()

    # Use the hashed data to update credentials
    global USERNAME, PWD
    # USERNAME = "mashraf_" + hashed_data[:8]  # Update username with a prefix
    # PWD = hashed_data[-16:]  # Use the last 16 characters for the password
    USERNAME = "mashraf"
    PWD = "Gttlleex1332"


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
        INVALID_APP_HASH = "2"

    def __init__(self, broker_address, port, client_id):
        self.broker_address = broker_address
        self.port = port
        self.client_id = client_id
        self.client = mqtt.Client(client_id)

        update_credentials()
        ssl_context = set_ssl()
        self.client.tls_set_context(ssl_context)

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
        else:
            mqtt_client.subscribe(SUB_TOPIC)

    def on_message(self, client, userdata, msg):
        # Extract the publisher's Client ID from the topic
        try:
            self.__mqtt_currmsg = msg.payload.decode("UTF-8")
        except UnicodeDecodeError:
            self.__mqtt_currmsg = msg.payload

        print(
            f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
        )
        self.__mqtt_msgsQ.put(str(self.__mqtt_currmsg))

    def connect(self):
        try:
            self.client.connect(self.broker_address, self.port, 60)
            print(
                f"Connected to broker_addr: `{BROKER_ADDR}` with client_id: `{CLIENT_ID}` succesffully | port: {PORT}"
            )
        except Exception as e:
            print(f"Connected error broker {BROKER_ADDR} error code: {e.error_code}")

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
        self.__sendToOEM(f"{SimpleMQTTClient.FotaVehicleResponse.NACK.value}")

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
        else:
            return False

        # TODO(Wx): Implement the one-shot receive mechanism
        #
        #
        # Start listening for the firmware hex line by line
        while True:
            firmware_msg = self.__mqtt_msgsQ.get()
            if firmware_msg == SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_DONE.value:
                self.__save_firmware(firmwareRx)
                self.__sendAck()
                break
            else:
                firmwareRx.append(str(firmware_msg))

        self.__vehicle_btl.cvtHex2Bin("./UpdatedFirmware.hex")
        calculatedHash = str(
            self.__vehicle_btl.calculate_hash_on_app_file("./UpdatedFirmware.bin")
        )

        # Start listening for the firmware hash
        self.__sendToOEM(SimpleMQTTClient.FotaVehicleResponse.SEND_FIRMWARE_HASH.value)
        if self.__mqtt_msgsQ.get() == SimpleMQTTClient.fotaOemResponse.OEM_ACK.value:
            pass
        else:
            return False

        while True:
            firmware_hash_msg = self.__mqtt_msgsQ.get()
            if (
                firmware_hash_msg
                == SimpleMQTTClient.FotaCommands.SEND_FIRMWARE_HASH_DONE.value
            ):
                if calculatedHash == firmwareHashRx:
                    self.__sendAck()
                    # print(f"Received hash: {firmwareHashRx}, Calculated hash: {calculatedHash}")
                    break
                else:
                    self.__sendNack(
                        SimpleMQTTClient.FotaErrorCode.INVALID_APP_HASH.value
                    )
                    print(
                        f"Invalid hash - Received hash: {firmwareHashRx}, Calculated hash: {calculatedHash}"
                    )
                    return SimpleMQTTClient.FotaErrorCode.INVALID_APP_HASH.value
            else:
                firmwareHashRx = str(firmware_hash_msg)

        # Save data
        self.__save_firmwareHash(firmwareHashRx)

        # End Of Comm
        self.__sendToOEM(SimpleMQTTClient.FotaVehicleResponse.DONE.value)

        return True

    def __save_firmware(self, firmware):
        with open("./UpdatedFirmware.hex", "wb") as f:
            for line in firmware:
                f.write(line.encode("utf-8") + b"\n")

    def __save_firmwareHash(self, firmwareHash):
        with open("UpdatedFirmwareHash.txt", "w") as f:
            f.write(firmwareHash)

    @staticmethod
    def __compress_file(input_path, output_path):
        with open(input_path, "rb") as file_in:
            with gzip.open(output_path, "wb") as file_out:
                file_out.writelines(file_in)

    @staticmethod
    def __decompress_file(input_path, output_path):
        with gzip.open(input_path, "rb") as file_in:
            with open(output_path, "wb") as file_out:
                file_out.write(file_in.read())

    def __vehicle_cmd_handle(self):
        if not self.__mqtt_msgsQ.empty():
            oem_msg = self.__mqtt_msgsQ.get()
            return self.__fota_cmd_exec(oem_msg)

    def run(self):
        self.__vehicle_btl = btl_ttl_intf(True, False)

        # Setup mqtt cfg
        mqtt_client.connect()
        time.sleep(1)  # Add a short delay after connecting

        mqtt_client.loop_start()

        status = False
        # Start the channel
        try:
            while True:
                try:
                    status = self.__vehicle_cmd_handle()
                    if status == True:
                        self.__vehicle_btl.set_serial_port("/dev/ttyUSB0")
                        self.__vehicle_btl.set_baudrate(115200)
                        #
                        if True == self.__vehicle_btl.update_firmware():
                            print(f"Flashed new software succesfully")
                        else:
                            print(f"Error occured while flashing the new software")
                    else:
                        pass
                except KeyboardInterrupt:
                    break
        finally:
            mqtt_client.loop_stop()
            mqtt_client.disconnect()


# Example usage:
if __name__ == "__main__":
    mqtt_client = SimpleMQTTClient(BROKER_ADDR, PORT, CLIENT_ID)
    mqtt_client.run()
