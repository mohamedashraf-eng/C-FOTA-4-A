"""
    @author Mohamed Ashraf Wx
    @date 2023 / 13 / 3
    @brief
    @copyright Copyright (c) Mohamed Ashraf, 2023 FOTA
    
    @attention

"""

import yaml
import json
import logging
import os
import queue
from enum import Enum
import time
import subprocess
import mysql.connector
import paho.mqtt.client as mqtt
import ssl

# Configure logging
log_filename = "main.log"
log_format = (
    "%(asctime)s - %(levelname)s - %(message)s - %(pathname)s:%(lineno)d - %(funcName)s"
)
logging.basicConfig(filename=log_filename, level=logging.INFO, format=log_format)


PASSIVE_RECONNECT_HANDLER = True


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


# MYSQL - CLASS
class prj_foem_mysql:
    def __init__(self, yaml_sql_cfg):
        self.__host = yaml_sql_cfg.get("host", "")
        self.__username = yaml_sql_cfg.get("username", "")
        self.__password = yaml_sql_cfg.get("password", "")
        self.__database = yaml_sql_cfg.get("database", "")
        self.__firmware_in_hex_fp = yaml_sql_cfg.get("firmware_in_hex_fp", "")
        self.__cnx = mysql.connector.connect(
            user=self.__username,
            password=self.__password,
            host=self.__host,
            database=self.__database,
        )
        self.__cursor = self.__cnx.cursor()

    def connect(self):
        self.__cnx = mysql.connector.connect(
            user=self.__username,
            password=self.__password,
            host=self.__host,
            database=self.__database,
        )
        self.__cursor = self.__cnx.cursor()

    def update(self, table_name, id_column, id_value, col, value):
        try:
            self.connect()
            update_stmt = f"UPDATE {table_name} SET {col} = %s WHERE {id_column} = %s"
            data = (value, id_value)
            self.__cursor.execute(update_stmt, data)
            self.__cnx.commit()
        except Exception as e:
            print(f"Error updating database: {e}")
        finally:
            self.close()

    def insert(self, table_name, value_dict):
        try:
            self.connect()
            columns = ", ".join(value_dict.keys())
            placeholders = ", ".join(["%s"] * len(value_dict))
            insert_query = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
            self.__cursor.execute(insert_query, tuple(value_dict.values()))
            self.__cnx.commit()
        except Exception as e:
            print(f"Error inserting into database: {e}")
        finally:
            self.close()

    def fetch(self, table_name, id_col, id, col):
        try:
            self.connect()
            fetch_query = f"SELECT {col} FROM {table_name} WHERE {id_col} = %s"
            self.__cursor.execute(fetch_query, (id,))
            data = self.__cursor.fetchone()
            if data is not None:
                return data[0]
            else:
                # Handle the case where no matching row was found
                return None
        except Exception as e:
            # Handle exceptions related to database connectivity or query execution
            print(f"Error fetching data: {str(e)}")
            return None
        finally:
            self.close()

    def fetch_single_value(self, query):
        self.connect()
        self.__cursor.execute(query)
        # fetchone() returns the first row of the results
        result = self.__cursor.fetchone()
        return result[0] if result else None

    def close(self):
        self.__cursor.close()
        self.__cnx.close()


#
#
#
#

# SSL/TLS
CLIENT_CERT = r"G:/WX_CAREER/Grad Project/src/vehicle_intf/testing/fotaHandler/mqtt/mqtt_certs/client/client2.crt"
CLIENT_KEY = r"G:/WX_CAREER/Grad Project/src/vehicle_intf/testing/fotaHandler/mqtt/mqtt_certs/client/client2.key"
CA_CERT = r"G:/WX_CAREER/Grad Project/src/vehicle_intf/testing/fotaHandler/mqtt/mqtt_certs/ca/ca.crt"
KEY_PASSPHRASE = "1332"

def set_ssl():
    ssl_context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    ssl_context.verify_mode = ssl.CERT_REQUIRED
    ssl_context.load_cert_chain(
        certfile=CLIENT_CERT, keyfile=CLIENT_KEY, password=KEY_PASSPHRASE
    )
    ssl_context.load_verify_locations(cafile=CA_CERT)
    return ssl_context

class oem_cmd(Enum):
    DEFAULT = 0xFFFFFFFF
    OEM_ACK = "40"
    OEM_NACK = "41"
    UPDATE_REQUEST = "1332"
    SEND_FIRMWARE_DONE = "50"
    SEND_FIRMWARE_HASH_DONE = "51"


class vehicle_cmd(Enum):
    VEHICLE_NACK = "70"
    VEHICLE_ACK = "69"
    DONE = "99"
    SEND_FIRMWARE = "30"
    SEND_FIRMWARE_HASH = "31"


def e_v(enum_):
    return enum_.value


#
# MQTT - CLASS


class prj_foem_mqtt:
    # Class methods

    def __init__(self, yaml_mqtt_cfg, yaml_mysql_cfg):
        if yaml_mqtt_cfg != None:
            self.__mqtt_cfg = yaml_mqtt_cfg
            logging.info(f"yaml mqtt cfg fetched")
        else:
            logging.error(f"no mqtt cfg provided.")
            raise ValueError(f"no mqtt cfg provided.")

        self.__mqtt_msgsQ = queue.Queue()
        #
        self.__mysql_obj = prj_foem_mysql(yaml_sql_cfg=yaml_mysql_cfg)
        self.__tablename = "website_mqtt"
        self.__mqtt_unlock_flag = True

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

    def __set_fotaRecord_status(self, status, fota_id):
        self.__mysql_obj.update(
            "website_fota_fota", "fota_id", fota_id, "status", status
        )
        
    def __set_fotaRecord_minutesTaken(self, minutes_taken, fota_id):
        self.__mysql_obj.update(
            "website_fota_fota", "fota_id", fota_id, "minutes_taken", minutes_taken
        )
   
    def __set_fotaRecord_startTime(self, started_at, fota_id):
        self.__mysql_obj.update(
            "website_fota_fota", "fota_id", fota_id, "started_at", started_at
        )
        
    def __set_fotaRecord_endTime(self, completed_at, fota_id):
        self.__mysql_obj.update(
            "website_fota_fota", "fota_id", fota_id, "completed_at", completed_at
        )
             
    def __save_sql_cfg(self):
        mqtt_cfg = {
            "broker": self.__brokeraddr,
            "port": self.__port,
            "client_id": self.__clientid,
            "username": self.__username,
            "password": self.__password,
            "ka_topic": self.__katopic,
            "pub_topic": self.__pubtopic,
            "sub_topic": self.__subtopic,
            "out_msg": "",
            "in_msg": "",
        }
        self.__mysql_obj.insert(self.__tablename, mqtt_cfg)

    def __set_pmqtt_cfg(self):
        # Paho-MQTT settings
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                logging.info(f"Successfully connected with code: 0")
                self.subscribe()
            else:
                logging.info(f"Connected with result code: `{str(rc)}`")
                # self.__attempt_reconnect()
                
        def on_disconnect(client, userdata, rc):
            logging.info(f"Disconnected with result code: `{str(rc)}`")

        def on_subscribe(client, userdata, mid, granted_qos):
            logging.info(f"subscribed to topic `{self.__subtopic}`")

        def on_unsubscribe(client, userdata, mid):
            logging.info(f"unsubscribed from topic `{self.__subtopic}`")

        def on_message(client, userdata, msg):
            # Extract the publisher's Client ID from the topic
            self.__mqtt_currmsg = msg.payload.decode("utf-8")
            logging.info(
                f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
            )
            print(
                f"Received message from topic: `{msg.topic}` msg: `{self.__mqtt_currmsg}`"
            )
            # Update value in mysql
            self.__mysql_obj.update(
                self.__tablename, "mqtt_id", 1, "in_msg", self.__mqtt_currmsg
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
        
        # ssl_context = set_ssl()
        # self.__pmqtt_client.tls_set_context(ssl_context)
        
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
            logging.info(
                f"connected to broker_addr: {self.__brokeraddr} with client_id: {self.__clientid} succesfully"
            )

    def __mqtt_pub_lock(self):
        self.__mqtt_unlock_flag = False

    def __mqtt_pub_unlock(self):
        self.__mqtt_unlock_flag = True

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
            # self.__attempt_reconnect()
        
    def disconnect(self):
        self.__pmqtt_client.disconnect()

    def publish(self, msg):
        assert msg is not None, f"msg must not be None value"
        self.__pmqtt_client.publish(
            topic=self.__pubtopic, qos=self.__qos, retain=self.__retain, payload=msg
        )
        logging.info(f"published: `{msg}` on topic `{self.__pubtopic}`")
        print(f"published: `{msg}` on topic `{self.__pubtopic}`")
        self.__mysql_obj.update(self.__tablename, "mqtt_id", 1, "out_msg", msg)

    def subscribe(self):
        logging.info(f"to subscribing to topic: `{self.__subtopic}`")
        self.__pmqtt_client.subscribe(topic=self.__subtopic, qos=self.__qos)
        logging.info(f"to subscribing to topic: `{self.__katopic}`")
        self.__pmqtt_client.subscribe(topic=self.__katopic, qos=self.__qos)

    def am_i_connected(self):
        return self.__pmqtt_client.is_connected()

    def __is_update_ready(self, last_active_job_id):
        return self.__mysql_obj.fetch(
            "website_fota_fota", "fota_id", last_active_job_id, "availability_flag"
        )

    #
    # COMM INTF: START
    # HANDLERS
    def __loadFirmwareToUpdate(self, fota_record_id):
        try:
            firmware_id = self.__mysql_obj.fetch(
                "website_fota_fota", "fota_id", fota_record_id, "firmware_id"
            )
            firmware = []
            BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            firmware_fp = self.__mysql_obj.fetch(
                "website_fota_firmware", "firmware_id", firmware_id, "firmware"
            )
            firmware_fp = os.path.join(BASE_DIR, firmware_fp)
            with open(firmware_fp, "r") as f:
                firmware = [line.strip() for line in f]

                return firmware
        except:
            raise Exception("Invalid firmware ID")

    def __loadFirmwareToUpdateHash(self, fota_record_id):
        try:
            firmware_id = self.__mysql_obj.fetch(
                "website_fota_fota", "fota_id", fota_record_id, "firmware_id"
            )

            firmware_hash = self.__mysql_obj.fetch(
                "website_fota_firmware", "firmware_id", firmware_id, "`firmware_cmac`"
            )

            return str(firmware_hash)
        except:
            raise Exception("Invalid firmware ID")

    def __setFotaRecordStatus(self, status):
        pass

    def __setFotaRecordStartTime(self, startTime):
        pass

    def __setFotaRecordEndtime(self, endtime):
        pass

    def __setFotaRecordTimeTaken(self, timeTaken):
        pass

    ##
    def __sendAck(self):
        self.publish(e_v(oem_cmd.OEM_ACK))

    def __sendNack(self):
        self.publish(e_v(oem_cmd.OEM_NACK))

    def __onVehicleCmd_UpdateRequestAck(self):
        start_time = time.monotonic()
        while (
            not self.__mqtt_msgsQ.empty()
            and self.__mqtt_msgsQ.queue[0] != e_v(vehicle_cmd.DONE)
        ):
            # Check for the vehicle SEND_FIRMWARE command
            # time.sleep(1)
            if self.__mqtt_msgsQ.get() == e_v(vehicle_cmd.SEND_FIRMWARE):
                self.__sendAck()
                #
                # time.sleep(1)
                #
                for line in self.__firmwareToUpload:
                    self.publish(line)
                    # time.sleep(0.2)
                self.publish(e_v(oem_cmd.SEND_FIRMWARE_DONE))
            
            while self.__mqtt_msgsQ.get() != e_v(vehicle_cmd.VEHICLE_ACK):
                pass
            
            if self.__mqtt_msgsQ.get() == e_v(vehicle_cmd.SEND_FIRMWARE_HASH):
                self.__sendAck()
                #
                # time.sleep(1)
                #
                self.publish(self.__firmwareToUploadHash)
                self.publish(e_v(oem_cmd.SEND_FIRMWARE_HASH_DONE))
                
            while self.__mqtt_msgsQ.get() != e_v(vehicle_cmd.VEHICLE_ACK):
                pass
        end_time = time.monotonic()
        self.__uploadingTimeTaken = (end_time - start_time)
        
        return True

    def __onVehicleCmd_UpdateRequestNack(self):
        print(f"{e_v(vehicle_cmd.VEHICLE_NACK)}")

        return True

    def __onVehicleCmd_IntegrityAck(self):
        pass

    def __onVehicleCmd_IntegrityNack(self):
        pass

    def __onVehicleCmd_AuthAck(self):
        pass

    def __onVehicleCmd_AuthNack(self):
        pass

    # TODO(Wx): Implement the commands handlers
    # MAIN HANDLERS
    def __vehicle_cmd_switch(self, cmd):
        return {
            #
            e_v(
                vehicle_cmd.VEHICLE_ACK
            ): lambda: self.__onVehicleCmd_UpdateRequestAck(),
            e_v(
                vehicle_cmd.VEHICLE_NACK
            ): lambda: self.__onVehicleCmd_UpdateRequestNack()
            #
        }.get(
            cmd,
            lambda: (
                logging.info(f"Invalid received cmd | {cmd}"),
                print(f"Invalid received cmd | {cmd}"),
            ),
        )()

    def __oem_cmd_handle(self):
        #
        # TODO(Wx): Implement the handler
        if not self.__mqtt_msgsQ.empty():
            vehicle_msg = self.__mqtt_msgsQ.get()
            return self.__vehicle_cmd_switch(vehicle_msg)
        else:
            pass

    #
    # COMM INTF: END
    # Main class sequence runner
    def run(self):
        # Fetch my configurations
        self.__get_cfg()
        # Set Paho-MQTT configurations
        self.__set_pmqtt_cfg()
        self.__last_id = None
        self.__last_active_job_id = None
        self.__job_done = False
        self.__job_delayed = False
        pending_records = []
        # Save first SQL
        # self.__save_sql_cfg()
        # Get hex file path
        # MAIN
        try:
            self.connect()
            self.loop_start()
            #
            while True:
                # TODO(Wx): Create an algorithm for processing multi fota records
                # ! IMPORTANT ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                # ! NOTE: For a simple demo, we will use only 1 record to test the functionality flow. (Abstraction)
                #
                # Fetch the highest ID in the fota_fota table
                curr_max_id = self.__mysql_obj.fetch_single_value(
                    f"SELECT MAX(fota_id) FROM website_fota_fota"
                )
                self.__last_active_job_id = curr_max_id
                record_status = str(
                    self.__mysql_obj.fetch(
                        "website_fota_fota", "fota_id", self.__last_active_job_id, "status"
                    )
                )
                self.__job_done = False
                self.__job_delayed = False
                # TODO(Wx): Change logic :'(
                if curr_max_id is not None and curr_max_id != self.__last_id:
                    self.__last_id = curr_max_id
                    if record_status == "completed":
                        self.__job_done = True
                    while (
                        self.__job_done != True
                        or self.__job_delayed != True
                    ):
                        curr_update_ready = self.__is_update_ready(
                            self.__last_active_job_id
                        )
                        #
                        if curr_update_ready == 1:
                            self.__firmwareToUpload = self.__loadFirmwareToUpdate(
                                self.__last_active_job_id
                            )
                            self.__firmwareToUploadHash = self.__loadFirmwareToUpdateHash(
                                self.__last_active_job_id
                            )
                            curr_job_status = self.__oem_cmd_handle()
                            # Start handle the incoming vehicle cmd
                            if True == curr_job_status:
                                self.__job_done = True
                                self.__set_fotaRecord_status("completed", self.__last_active_job_id)
                                self.__set_fotaRecord_minutesTaken(self.__uploadingTimeTaken, self.__last_active_job_id)
                                self.__set_fotaRecord_startTime("X", self.__last_active_job_id)
                                self.__set_fotaRecord_endTime("Y", self.__last_active_job_id)
                                break
                            if self.__mqtt_unlock_flag:
                                self.publish(f"{ e_v(oem_cmd.UPDATE_REQUEST) }")
                        else:
                            if self.__mqtt_unlock_flag:
                                self.publish(f"{ e_v(oem_cmd.DEFAULT) }")
                            # Base listner
                            # self.__oem_cmd_handle()
                        time.sleep(2)
                    #
                    self.__last_active_job_id = self.__last_id
                else:
                    print(f"There is no new FOTA record. last@{self.__last_id}")
                #
                time.sleep(5)
        except KeyboardInterrupt:
            self.disconnect()
            self.loop_stop()
            self.__mysql_obj.close()


if __name__ == "__main__":
    myyaml = prj_foem_yaml("main.yaml")
    myyaml.run()

    mymqtt = prj_foem_mqtt(
        yaml_mqtt_cfg=myyaml.get_mqttcfg, yaml_mysql_cfg=myyaml.get_sqlcfg
    )

    mymqtt.run()
    # TODO(Wx): Make a recovery handle
    try:
        if PASSIVE_RECONNECT_HANDLER:
            # Passive handle in (testing)
            if not mymqtt.am_i_connected():
                mymqtt.run()
        else:
            # Offenseve handle in (production)
            while not mymqtt.am_i_connected():
                mymqtt.run()
    except Exception as e:
        # TODO(Wx): Critical Exception procedure
        raise Exception(
            "Unkown exception error due to undefined behaviour. [CRITICAL ! ! ]"
        )
