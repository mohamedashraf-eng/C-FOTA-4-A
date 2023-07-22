# Base libs
import logging
import os
import queue
import subprocess
import time
import yaml
import json

# Proj libs
import pkg_resources
import sqlite3 as sql
import paho.mqtt.client as mqtt
import hashlib

# Configure logging
log_filename = "main.log"
log_format = (
    "%(asctime)s - %(levelname)s - %(message)s - %(pathname)s:%(lineno)d - %(funcName)s"
)
logging.basicConfig(filename=log_filename, level=logging.INFO, format=log_format)

# YAML - CLASS
class prj_foem_yaml:
    # Class methods
    def __init__(self, filepath):
        assert filepath is not None, f"file path must be specified"
        self.__filepath = filepath
        self.__filename = os.path.basename(filepath)

    def __repr__(self):
        return f"prj_fdb1_yaml(yaml_filepath={self.__filepath})"

    def __str__(self):
        return f"class for parsing yaml file for project `fdb1`"

    # Private methods
    def __parse_yaml_file(self):
        with open(self.__filepath) as yaml_file:
            yaml_data = yaml.safe_load(yaml_file)
        return yaml_data

    def __get_version(self, yaml_data) -> str:
        self.__version = yaml_data["version"]
        logging.info(f"yaml v{self.__version}")
        return self.__version

    def __get_dependencies(self, yaml_data) -> None:
        self.__p_dependencies = yaml_data.get("dependencies", [])
        # Check for dependencies
        if self.__p_dependencies:
            for package in self.__p_dependencies:
                try:
                    pkg_resources.get_distribution(package.split("==")[0])
                    logging.info(f"{package} is already installed.")
                except:
                    try:
                        logging.info(f"Installing {package}...")
                        subprocess.check_call(["pip", "install", package])
                        logging.info(f"{package} installed successfully.")
                    except subprocess.CalledProcessError:
                        logging.error(f"Failed to install {package}.")

    def __get_mqttcfg(self, yaml_data=None) -> dict:
        self.__p_mqttcfg = yaml_data.get("mqtt_cfg", {})
        # Return the MQTT configuration data as a dictionary
        return self.__p_mqttcfg

    def __get_sqlcfg(self, yaml_data=None) -> dict:
        self.__p_sqlcfg = yaml_data.get("sql_cfg", {})
        return self.__p_sqlcfg

    def __get_cmdcfg(self, yaml_data=None) -> dict:
        self.__p_cmdcfg = yaml_data.get("cmd_cfg", {})
        return self.__p_cmdcfg

    # Public methods
    @property
    def get_version(self):
        return self.__get_version(self.__yaml_data)

    @property
    def get_mqttcfg(self):
        return self.__get_mqttcfg(self.__yaml_data)

    @property
    def get_sqlcfg(self):
        return self.__get_sqlcfg(self.__yaml_data)

    @property
    def get_cmdcfg(self):
        return self.__get_cmdcfg(self.__yaml_data)

    # Main class sequence runner
    def run(self):
        # Install the dependencies
        self.__yaml_data = self.__parse_yaml_file()
        self.__get_dependencies(self.__yaml_data)


# MQTT - CLASS
class prj_foem_mqtt:
    # Class methods
    def __init__(self, manual_mqtt_cfg, yaml_mqtt_cfg):
        if yaml_mqtt_cfg != None:
            self.__mqtt_cfg = yaml_mqtt_cfg
            logging.info(f"yaml mqtt cfg fetched")
        elif manual_mqtt_cfg != None:
            self.__mqtt_cfg = manual_mqtt_cfg
            logging.info(f"manual mqtt cfg fetched")
            # TODO(wx): Finish it
            pass
        else:
            logging.error(f"no mqtt cfg provided.")
            raise ValueError(f"no mqtt cfg provided.")

        self.__mqtt_msgsQ = queue.Queue()

    def __repr__(self):
        pass

    def __str__(self):
        return f"class for mqtt connection"

    # Private methods
    def __get_cfg(self):
        # Log all the configurations
        for key, value in self.__mqtt_cfg.items():
            if key == "pub_topic" or key == "sub_topic":
                assert value != "", f"value must be a valid topic"
            logging.info(f"{key}: {value}")
        # Add the configurations
        self.__brokeraddr = self.__mqtt_cfg.get("broker_addr", "")
        self.__port = self.__mqtt_cfg.get("port", 0)
        self.__clientid = self.__mqtt_cfg.get("client_id", "")
        self.__username = self.__mqtt_cfg.get("username", "")
        self.__password = self.__mqtt_cfg.get("password", "")
        self.__pubtopic = self.__mqtt_cfg.get("pub_topic", "")
        self.__subtopic = self.__mqtt_cfg.get("sub_topic", "")
        self.__katopic = self.__mqtt_cfg.get("ka_topic", "")
        self.__keepalive = self.__mqtt_cfg.get("keepalive", 0)
        self.__timeout = self.__mqtt_cfg.get("timeout", 0)
        self.__qos = self.__mqtt_cfg.get("qos", 0)
        self.__encryption = self.__mqtt_cfg.get("encryption", False)
        self.__retain = self.__mqtt_cfg.get("retain", False)
        logging.info(f"mqtt configuration fetched.")

    def __set_pmqtt_cfg(self):
        # Paho-MQTT settings
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                logging.info(f"Successfully connected with code: 0")
                self.subscribe()
            else:
                logging.info(f"Connected with result code: `{str(rc)}`")
                self.__attempt_reconnect()

        def on_disconnect(client, userdata, rc):
            logging.info(f"Disconnected with result code: `{str(rc)}`")

        def on_subscribe(client, userdata, mid, granted_qos):
            logging.info(f"subscribed to topic `{self.__subtopic}`")

        def on_unsubscribe(client, userdata, mid):
            logging.info(f"unsubscribed from topic `{self.__subtopic}`")

        def on_message(client, userdata, msg):
            # Extract the publisher's Client ID from the topic
            self.__mqtt_currmsg = msg.payload.decode("UTF-8")
            logging.info(
                f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
            )
            print(
                f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
            )
            # Logging message in a queue
            self.__mqtt_msgsQ.put(str(self.__mqtt_currmsg))

        self.__pmqtt_client = mqtt.Client()
        self.__pmqtt_client.on_connect = on_connect
        self.__pmqtt_client.on_disconnect = on_disconnect
        self.__pmqtt_client.on_message = on_message

        self.__pmqtt_client.username_pw_set(
            username=self.__username, password=self.__password
        )

    def __get_msgsQ(self):
        return self.__mqtt_msgsQ

    def __attempt_reconnect(self):
        retries = 0
        max_retries = 5
        delay_interval_s = 2

        while not self.__pmqtt_client.is_connected() and retries <= max_retries:
            logging.info(f"Attempting reconnection, retry {retries+1}/{max_retries}")
            self.connect()
            time.sleep(delay_interval_s)
            retries += 1
            # Exponential backoff, doubling the delay for each retry
            delay_interval_s *= 2
        # After attempting
        if not self.__pmqtt_client.is_connected():
            logging.critical(
                f"Failed to reconnect | client_id: {self.__clientid} | broker_addr: {self.__brokeraddr}"
            )
        else:
            logging.critical(
                f"connected to broker_addr: {self.__brokeraddr} with client_id: {self.__clientid} succesfully"
            )

    # Public methods
    def loop_start(self):
        self.__pmqtt_client.loop_start()

    def loop_stop(self):
        self.__pmqtt_client.loop_stop()

    def connect(self):
        try:
            self.__pmqtt_client.connect(
                self.__brokeraddr, self.__port, self.__keepalive
            )
            print(
                f"connected to broker_addr: `{self.__brokeraddr}` with client_id: `{self.__clientid}` succesfully | port: `{self.__port}`"
            )
            logging.info(
                f"connected to broker_addr: `{self.__brokeraddr}` with client_id: `{self.__clientid}` succesfully | port: `{self.__port}`"
            )
        except Exception as e:
            logging.error(
                f"Connection error broker `{self.__brokeraddr}` | code: `{str(e)}`"
            )
            # Handler mechanism
            logging.info(f"Starting failure handler - calling; `__attempt_reconnect`")
            self.__attempt_reconnect()

    def disconnect(self):
        self.__pmqtt_client.disconnect()

    def publish(self, msg: str = "ashraf"):
        logging.info(f"published: `{msg}` on topic `{self.__pubtopic}`")
        print(f"published: `{msg}` on topic `{self.__pubtopic}`")
        self.__pmqtt_client.publish(
            topic=self.__pubtopic, qos=self.__qos, retain=self.__retain, payload=msg
        )

    def subscribe(self):
        logging.info(f"to subscribing to topic: `{self.__subtopic}`")
        self.__pmqtt_client.subscribe(topic=self.__subtopic, qos=self.__qos)
        logging.info(f"to subscribing to topic: `{self.__katopic}`")
        self.__pmqtt_client.subscribe(topic=self.__katopic, qos=self.__qos)

    @property
    def msgsQ(self):
        return self.__get_msgsQ()

    # Main class sequence runner
    def run(self):
        # Fetch my configurations
        self.__get_cfg()
        # Set Paho-MQTT configurations
        self.__set_pmqtt_cfg()
        # Dummy Demo
        # TODO(wx): Change logic for the mqtt listener
        try:
            self.connect()
            self.loop_start()

            while True:
                self.publish("Hello from delta foem v1.0.0")
                time.sleep(3)
        except KeyboardInterrupt:
            self.disconnect()
            self.loop_stop()


# SQL - CLASS
class prj_foem_sqlite:
    # Class methods
    def __init__(self, yaml_sql_cfg):
        assert yaml_sql_cfg is not None, f"yaml_sql_cfg must not be None"
        self.__sqlcfg = yaml_sql_cfg
        self.__db_fp = ""

    def __repr__(self):
        pass

    def __str__(self):
        return f"class for SQL"

    # Private methods
    def __fetch_cfg(self):
        self.__db_path = self.__sqlcfg.get("db_path", "")
        self.__db_name = self.__sqlcfg.get("db_name", "")
        self.__db_ecuschema = self.__sqlcfg.get("ecu_schema", "")
        self.__db_firmwareschema = self.__sqlcfg.get("firmware_schema", "")
        self.__db_vehicleschema = self.__sqlcfg.get("vehicle_schema", "")
        self.__db_fotaschema = self.__sqlcfg.get("fota_schema", "")
        self.__db_fp = os.path.join(self.__db_path, self.__db_name)
        self.__firmware_hash_type = self.__sqlcfg.get("firmware_hash_type", "")
        logging.info("fetched sqlite configuration succefully.")

    def __connect_db(self):
        self.__db_connector = sql.connect(self.__db_fp)
        logging.info(f"connected to {self.__db_fp} successfully")

    def __close_db(self):
        self.__db_connector.close()
        logging.info(f"disconnected from {self.__db_fp} successfully")

    def __create_db(self):
        # Create a new db
        self.__db_cursor = self.__db_connector.cursor()
        self.__db_cursor.execute(self.__db_ecuschema)
        self.__db_cursor.execute(self.__db_firmwareschema)
        self.__db_cursor.execute(self.__db_vehicleschema)
        self.__db_cursor.execute(self.__db_fotaschema)
        self.__db_connector.commit()
        logging.info(f"Database {self.__db_fp} created successfully!")

    def __set_current_id(self, id: int):
        logging.info(f"setting current_id: {id}")
        self.__table_id = id

    def __insert_new_row_in_table(self, table_name: str, **values):
        self.__connect_db()

        column_names = ", ".join(values.keys())
        value_placeholders = ", ".join(["?" for _ in values])

        insert_query = f"""
            INSERT INTO "{table_name}" ({column_names}) VALUES ({value_placeholders})
        """

        values_to_insert = tuple(values.values())

        self.__db_cursor.execute(insert_query, values_to_insert)
        self.__close_db()
        logging.info(f"Inserted a new row in table {table_name}")

    def __fetch_from_table(self, table_name: str, column_name: str, row_id: int):
        self.__connect_db()

        select_query = f"""
            SELECT {column_name} FROM "{table_name}" WHERE id=?
        """

        self.__db_cursor.execute(select_query, (row_id,))
        row = self.__db_cursor.fetchall()

        self.__close_db()
        logging.info(
            f"Fetched from table {table_name} column {column_name}, row_id={row_id}"
        )
        return row

    def __get_row_in_table(self, table_name: str, row_id: int):
        self.__connect_db()
        row = self.__fetch_from_table(table_name, "*", row_id)
        self.__close_db()
        logging.info(f"Fetched row from table {table_name} column, row_id={row_id}")
        return row

    def __get_value_from_table(self, row_id: int, table_name: str, column_name: str):
        self.__connect_db()
        data = self.__fetch_from_table(table_name, column_name, row_id)
        self.__close_db()
        logging.info(
            f"Fetched value from table {table_name} colummn {column_name}, row_id={row_id}"
        )
        return self.__raw(data)

    def __set_value_in_table(
        self, row_id: int, table_name: str, column_name: str, value
    ):
        self.__connect_db()
        self.__set_current_id(row_id)
        # Update the specific column value with the provided value
        update_query = f'UPDATE {table_name} SET "{column_name}"=? WHERE "id"=?'
        self.__db_cursor.execute(update_query, (value, row_id))
        self.__close_db()
        logging.info(
            f"Setted value to table {table_name} colummn {column_name}, row_id={row_id} | value {value}"
        )

    def __print_table(self, table_name):
        self.__db_cursor.execute(f"SELECT * FROM {table_name}")
        rows = self.__db_cursor.fetchall()
        for row in rows:
            print(row)

        logging.info(f"table {table_name} printed")

    @staticmethod
    def __raw(fetched):
        return fetched[0][0]

    def run(self):
        self.__connect_db()
        self.__fetch_cfg()
        self.__create_db()


# SQLite3 - Inh - firmware
class prj_foem_sqlite_firmware(prj_foem_sqlite):
    def __init__(self, firmware_fp, yaml_sql_cfg):
        # super cfg
        super().__init__(yaml_sql_cfg)
        super().run()
        # child cfg
        self.__firmware_fp = firmware_fp
        self.__firmware_hex = []
        self.__firmware_in_json = None
        self.__firmware_hash = None
        self.__firmware_size = None
        self.__table_name = "firmware_data"
        self.__row_id = 1

        self.__firmware_hex_fetch()
        self.__firmware_in_json = json.dumps(self.__firmware_hex)

    def __firmware_hex_fetch(self):
        try:
            with open(self.__firmware_fp, "r") as hex_file:
                self.__firmware_hex.append(hex_file.readlines())
            logging.info(f"Fetched the hex file successfully.")
        except FileNotFoundError:
            logging.error(f"Error: File '{self.__firmware_fp}' not found.")
            return None

    def __firmware_cvt2bin(self):
        # Sup function
        def hex_to_binary(hex_string):
            # Function to convert a string of hex characters to binary data
            hex_string = "".join(
                c for c in hex_string if c.isdigit() or c.lower() in "abcdef"
            )
            if len(hex_string) % 2 != 0:
                hex_string = "0" + hex_string
            return bytes.fromhex(hex_string)

        try:
            with open(self.__firmware_fp, "r") as hex_file:
                hex_data = hex_file.read().replace("\n", "")
                binary_data = hex_to_binary(hex_data)
                with open(
                    f"{os.path.splitext(self.__firmware_fp)[0]}.bin", "wb"
                ) as binary_file:
                    binary_file.write(binary_data)
            logging.info(
                f"Conversion successful. Binary file created:{os.path.splitext(self.__firmware_fp)[0]}.bin"
            )

            # Calculate SHA-256 hash of the binary data
            sha256_hash = hashlib.sha256(binary_data).hexdigest()
            self.__firmware_hash = sha256_hash
            logging.info(
                f"created a hash [SHA2-256] for firmware {self.__firmware_hash}"
            )
        except FileNotFoundError:
            logging.error("Error: The specified hex file was not found.")

    def __firmware_calculate_size(self):
        try:
            with open(self.__firmware_fp, "r") as hex_file:
                firmware_size = 0
                for line in hex_file:
                    # Remove newline characters and any leading/trailing whitespaces
                    hex_line = line.strip()
                    # Each line has a record type in the last two characters, which we exclude from size calculation
                    if hex_line and hex_line[-2:] != "00":
                        # Each byte is represented by two hex digits, so increment size by half the hex length
                        firmware_size += len(hex_line) // 2

                self.__firmware_size = firmware_size

        except FileNotFoundError:
            logging.error(f"Error: File '{self.__firmware_fp}' not found.")
            return None

    def __firmware_calculate_hash(self):
        # Read the hex file
        with open(self.__firmware_fp, "r") as hex_file:
            hex_data = hex_file.readlines()

        # Remove non-hexadecimal characters from each line
        hex_data_sanitized = [line.strip().replace(" ", "") for line in hex_data]

        # Calculate the SHA256 hash
        sha256_hash = hashlib.sha256()
        for hex_line in hex_data_sanitized:
            bytes_line = bytes.fromhex(hex_line)
            sha256_hash.update(bytes_line)

        # Get the final hash in hexadecimal format
        firmware_hash = sha256_hash.hexdigest()
        self.__hash = firmware_hash.upper()  # Convert to uppercase for consistency
        logging.error(f"Error: File '{self.__firmware_fp}' not found.")

    def insert_row(self, **row):
        assert row is not None, f"row should not be None"
        self._prj_foem_sqlite__insert_new_row_in_table(self.__table_name, **row)

    def get_row(self):
        self._prj_foem_sqlite__get_row_in_table(self.__table_name, self.__row_id)

    @property
    def version(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "version"
        )
        return data

    @version.setter
    def version(self, version: str):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "version", version
        )

    @property
    def firmware_in_hex(self):
        data = json.loads(
            self._prj_foem_sqlite__get_value_from_table(
                self.__row_id, self.__table_name, "firmware_in_hex"
            )
        )
        return data

    @firmware_in_hex.setter
    def firmware_in_hex(self, firmware_in_hex: list):
        firmware_in_hex = json.dumps(firmware_in_hex)
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "firmware_in_hex", firmware_in_hex
        )

    @property
    def update_size(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_size"
        )
        return data

    @update_size.setter
    def update_size(self, update_size: int):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_size", update_size
        )

    @property
    def update_hash(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_hash"
        )
        return data

    @update_hash.setter
    def update_hash(self, update_hash: str):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_hash", update_hash
        )

    @property
    def update_notes(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_notes"
        )
        return data

    @update_notes.setter
    def update_notes(self, update_notes: str):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_notes", update_notes
        )

    def print_table(self):
        return self._prj_foem_sqlite__print_table(self.__table_name)

    def run(self):
        self.__firmware_calculate_size()
        self.__firmware_cvt2bin()

        # Test
        myquery = {
            "version": "1.0.0",
            "firmware_in_hex": "self.__firmware_in_json",
            "update_size": self.__firmware_size,
            "update_hash": self.__firmware_hash,
            "update_notes": "nothing",
        }

        self.insert_row(**myquery)

        self.print_table()


# SQLite3 - Inh - vehicle
class prj_foem_sqlite_vehicle(prj_foem_sqlite):
    all = []

    def __init__(self, yaml_sql_cfg):
        # super cfg
        super().__init__(yaml_sql_cfg)
        super().run()
        # child cfg
        self.__table_name = "vehicle_data"
        self.__row_id = 1

    def insert_row(self, **values):
        self._prj_foem_sqlite__insert_new_row_in_table(self.__table_name, **values)

    def get_row(self):
        self._prj_foem_sqlite__get_row_in_table(self.__table_name, self.__row_id)

    @property
    def vehicle_id(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "vehicle_id"
        )
        return data

    @vehicle_id.setter
    def vehicle_id(self, vehicle_id):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "vehicle_id", vehicle_id
        )

    @property
    def vehicle_name(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "vehicle_name"
        )
        return data

    @vehicle_name.setter
    def vehicle_name(self, vehicle_name):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "vehicle_name", vehicle_name
        )

    @property
    def vehicle_type(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "vehicle_type"
        )
        return data

    @vehicle_type.setter
    def vehicle_type(self, vehicle_type):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "vehicle_type", vehicle_type
        )

    @property
    def vehicle_status(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "vehicle_status"
        )
        return data

    @vehicle_status.setter
    def vehicle_status(self, vehicle_status):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "vehicle_status", vehicle_status
        )


# SQLite3 - Inh - ECU's
class prj_foem_sqlite_ecu(prj_foem_sqlite):
    all = []

    def __init__(self, yaml_cfg_file):
        # super cfg
        super().__init__(yaml_cfg_file)
        super().run()
        # child cfg
        self.__row_id = 1
        self.__table_name = "ecu_data"


# SQLite3 - Inh - fota
class prj_foem_sqlite_fota(prj_foem_sqlite):
    all = []

    def __init__(self, yaml_sql_cfg):
        # super cfg
        super().__init__(yaml_sql_cfg)
        super().run()
        # child cfg
        self.__table_name = "fota_data"
        self.__row_id = 1

    @property
    def vehicle_id(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "vehicle_id"
        )
        return data

    @vehicle_id.setter
    def vehicle_id(self, vehicle_id: int):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "vehicle_id", vehicle_id
        )

    @property
    def firmware_id(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "firmware_id"
        )
        return data

    @firmware_id.setter
    def firmware_id(self, firmware_id: int):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "firmware_id", firmware_id
        )

    @property
    def ecu_id(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "ecu_id"
        )
        return data

    @ecu_id.setter
    def ecu_id(self, ecu_id: str):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "ecu_id", ecu_id
        )

    @property
    def update_available(self):

        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_available"
        )
        return data

    @update_available.setter
    def update_available(self, update_available: int):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_available", update_available
        )

    @property
    def update_status(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_status"
        )
        return data

    @update_status.setter
    def update_status(self, update_status: str):

        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_status", update_status
        )

    @property
    def last_update_time(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "last_update_time"
        )
        return data

    @last_update_time.setter
    def last_update_time(self, last_update_time: str):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "last_update_time", last_update_time
        )

    @property
    def update_success(self):
        data = self._prj_foem_sqlite__get_value_from_table(
            self.__row_id, self.__table_name, "update_success"
        )
        return data

    @update_success.setter
    def update_success(self, update_success: int):
        self._prj_foem_sqlite__set_value_in_table(
            self.__row_id, self.__table_name, "update_success", update_success
        )


# CMD_ANALYZER - CLASS:
class prj_foem_cmd:
    # Class methods
    def __init__(self, yaml_cmd_cfg):
        assert yaml_cmd_cfg is not None, "yaml_cmd_cfg must not be None"
        self.__cmdcfg = yaml_cmd_cfg
        self.__cmdQ = None

    def __repr__(self):
        return f"prj_foem_cmd()"

    def __str__(self):
        return f"class to analyize messages as commands and execute it"

    # Private methods
    def __get_cmdcfg(self):
        # TODO(wx): Change it to the planned
        for key, value in self.__cmdcfg.items():
            logging.info(f"{key}: {value}")
        self.__getters = self.__cmdcfg.get("getters", [])
        self.__setters = self.__cmdcfg.get("setters", [])
        self.__requests = self.__cmdcfg.get("requests", [])

    # Public methods
    @property
    def cmd_msgsQ(self):
        return self.__cmdQ

    @cmd_msgsQ.setter
    def cmd_msgsQ(self, mqtt_msgs_Q):
        assert mqtt_msgs_Q is not None, "mqtt msgs queue mustn't be None"
        self.__cmdQ = mqtt_msgs_Q

    def run(self):
        self.__get_cmdcfg()


# Main function entry point
def main():
    # Yaml
    main_yaml = prj_foem_yaml("main.yaml")
    main_yaml.run()
    # SQL
    main_ecu = prj_foem_sqlite_ecu(main_yaml.get_sqlcfg)
    main_firmware = prj_foem_sqlite_firmware("test.hex", main_yaml.get_sqlcfg)
    main_firmware.run()
    main_vehicle = prj_foem_sqlite_vehicle(main_yaml.get_sqlcfg)
    main_fota = prj_foem_sqlite_fota(main_yaml.get_sqlcfg)
    # MQTT
    main_mqtt = prj_foem_mqtt(yaml_mqtt_cfg=main_yaml.get_mqttcfg, manual_mqtt_cfg=None)
    # main_mqtt.run()
    # CMD_ANALYZER
    main_cmd = prj_foem_cmd(yaml_cmd_cfg=main_yaml.get_cmdcfg)
    main_cmd.cmd_msgsQ = main_mqtt.msgsQ
    main_cmd.run()


# Main thread
if __name__ == "__main__":
    main()
else:
    print("Not running as a standalone script.")
