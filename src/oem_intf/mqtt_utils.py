import logging
import os
import queue
# import yaml import subprocess
import time
import yaml
import pkg_resources
import paho.mqtt.client as mqtt

# Configure logging
log_filename = "main.log"
log_format = (
    "%(asctime)s - %(levelname)s - %(message)s - %(pathname)s:%(lineno)d - %(funcName)s"
)
logging.basicConfig(filename=log_filename,
                    level=logging.INFO, format=log_format)

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
    def __init__(self, yaml_mqtt_cfg):
        if yaml_mqtt_cfg != None:
            self.__mqtt_cfg = yaml_mqtt_cfg
            logging.info(f"yaml mqtt cfg fetched")
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
            logging.info(
                f"Attempting reconnection, retry {retries+1}/{max_retries}")
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
            logging.info(
                f"Starting failure handler - calling; `__attempt_reconnect`")
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


if __name__ == '__main__':
    myYaml = prj_foem_yaml('main.yaml')
    myYaml.run()
    myMqtt = prj_foem_mqtt(yaml_mqtt_cfg=myYaml.get_mqttcfg)
    myMqtt.run()
