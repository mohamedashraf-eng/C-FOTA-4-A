import serial
import serial.tools.list_ports
import struct
import time
import sys
import os
import logging
from enum import Enum
from intelhex import IntelHex

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s]: %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)

OUTPUT_BIN_FILE_STD = ""
DUMMY_APP_BIN_FP = r"G:\WX_CAREER\Grad Project\src\vehicle_intf\testing\TestAppLedOn\MDK-ARM\TestAppLedOn\TestAppLedOn.bin"

class btl_ttl_intf(object):
    class BtlCommands(Enum):
        NUM_OF_CMDS = 13
        CBL_GET_VER_CMD = 0
        CBL_GET_HELP_CMD = 1
        CBL_GET_CID_CMD = 2
        CBL_GET_RDP_STATUS_CMD = 3
        CBL_GO_TO_ADDR_CMD = 4
        CBL_FLASH_ERASE_CMD = 5
        CBL_MEM_WRITE_CMD = 6
        CBL_EN_R_W_PROTECT_CMD = 7
        CBL_MEM_READ_CMD = 8
        CBL_OTP_READ_CMD = 9
        CBL_DIS_R_W_PROTECT_CMD = 10
        CBL_READ_SECTOR_STATUS = 11
        CBL_GET_INFO_CMD = 12
        CBL_FLASH_APP_CMD = 13
        CBL_SW_RESET = 14

    std_baudrates = [9600, 19200, 38400, 57600, 115200, 1500000]

    MAX_DATA_PER_PACKET_LENGTH = 100

    def __init__(self, verbose_mode=False, mem_wrt_actv=False):
        self.__verbose_mode = verbose_mode
        self.__mem_wrt_actv = mem_wrt_actv
        self.__btl_available_commands = {
            "CBL_GET_VER_CMD": "Get Bootloader Version",
            "CBL_GET_HELP_CMD": "Get Help Information",
            "CBL_GET_CID_CMD": "Get Chip ID",
            "CBL_GET_RDP_STATUS_CMD": "Get Read Protection Status",
            "CBL_GO_TO_ADDR_CMD": "Go to Specified Address",
            "CBL_FLASH_ERASE_CMD": "Erase Flash Memory",
            "CBL_MEM_WRITE_CMD": "Write to Memory",
            "CBL_EN_R_W_PROTECT_CMD": "Enable Read/Write Protection",
            "CBL_MEM_READ_CMD": "Read from Memory",
            "CBL_OTP_READ_CMD": "Read from OTP Memory",
            "CBL_DIS_R_W_PROTECT_CMD": "Disable Read/Write Protection",
            "CBL_READ_SECTOR_STATUS": "Read Sector Status",
            "CBL_GET_INFO_CMD": "Get Bootloader Information",
            "CBL_FLASH_APP_CMD": "Flashing the application binary",
            "CBL_SW_RESET": "Seneding a reset signal to the bootloader",
        }
        self.__btl_commands = {
            # btl_ttl_intf.BtlCommands.CBL_GET_VER_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_GET_HELP_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_GET_CID_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_GET_RDP_STATUS_CMD:,
            btl_ttl_intf.BtlCommands.CBL_GO_TO_ADDR_CMD.value: self.btl_cmd_intf_JumpToAddr,
            btl_ttl_intf.BtlCommands.CBL_FLASH_ERASE_CMD.value: self.btl_cmd_intf_EraseFlash,
            btl_ttl_intf.BtlCommands.CBL_FLASH_APP_CMD.value: self.btl_cmd_intf_FlashApp,
            # btl_ttl_intf.BtlCommands.CBL_MEM_WRITE_CMD: ,
            # btl_ttl_intf.BtlCommands.CBL_EN_R_W_PROTECT_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_MEM_READ_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_OTP_READ_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_DIS_R_W_PROTECT_CMD:,
            # btl_ttl_intf.BtlCommands.CBL_READ_SECTOR_STATUS:,
            # btl_ttl_intf.BtlCommands.CBL_GET_INFO_CMD:,
            btl_ttl_intf.BtlCommands.CBL_SW_RESET.value: self.btl_cmd_intf_Reboot,
            # ... add other commands here
        }

        self.__serial_port = 0
        self.__baudrate = 0
        self.__timeout = 0

        self.__application_bin_fp = DUMMY_APP_BIN_FP

    def __available_ports(self):
        ports = serial.tools.list_ports.comports()
        if not ports:
            logging.error("No serial ports found.")
        else:
            logging.info("Available serial ports:")
            for port, desc, hwid in sorted(ports):
                logging.info(f"- {port}: {desc} [{hwid}]")

    def __check_serial_port(self, port):
        available_ports = [p.device for p in serial.tools.list_ports.comports()]
        return port in available_ports

    def __connect_to_port(self, port, br=115200, timeout=None):
        try:
            if self.__check_serial_port(port):
                self.__serialObj = serial.Serial(
                    port=port,
                    baudrate=br,
                    bytesize=8,
                    parity="N",
                    stopbits=1,
                    timeout=timeout,
                )
                time.sleep(1)
                if self.__serialObj.is_open:
                    logging.info(f"Connected to port {port}@{br}:N:1 successfully")
                    return True
                else:
                    logging.error(f"Failed to open port {port}")
                    return False
            else:
                logging.error(f"Port {port} does not exist or is not available")
                self.__available_ports()
                return False
        except Exception as e:
            logging.error(f"Error connecting to port {port}: {str(e)}")
            return False

    def __write_to_serial_until(self, Value, Length):
        _data = struct.pack(">B", Value)
        if self.__verbose_mode:
            Value = bytearray(_data)
            print("   " + "0x{:02x}".format(Value[0]), end=" ")
            if self.__mem_wrt_actv and (not self.__verbose_mode):
                logging.info("#", end=" ")
            self.__serialObj.write(_data)

    def __read_from_serial_until(self, DataLength, WaitingMsg: str = None):
        Serial_Value = self.__serialObj.read(DataLength)
        Serial_Value_len = len(Serial_Value)
        while Serial_Value_len <= 0:
            Serial_Value = self.__serialObj.read(DataLength)
            Serial_Value_len = len(Serial_Value)
            if WaitingMsg is not None:
                print(WaitingMsg)
        return Serial_Value

    """bootloaderpacket_format:
            [Packet Length] [Packet Type] [Command] [Data Length] [Data]    [Data CRC32] [Packet CRC32]
            [1 Byte]        [1 Byte]      [1 Bytte] [1 Byte]      [n Bytes] [4 Bytes]    [4 Bytes]
    """

    def __encode_btl_packet(self, packetDict):
        if packetDict is not None:
            packetList = []
            #
            PacketType = packetDict["PacketType"]
            Command = packetDict["Command"]
            #
            data = packetDict["Data"]
            dataLen = len(data)
            dataCRC32 = self.__calculate_crc32_04C11DB7(data, dataLen)
            #
            packetList.append(PacketType)
            packetList.append(Command)
            packetList.append(dataLen)
            packetList = packetList[:5] + data + packetList[5:]

            # Calculate Data CRC32 and insert into packetList
            for i in range(4):
                packetList.insert(
                    dataLen + 4 + i, self.__cvt_value2byte(dataCRC32, i + 1, 1)
                )

            # Calculate Packet CRC32 and insert into packetList
            packetCRC32 = self.__calculate_crc32_04C11DB7(packetList, len(packetList))
            for i in range(4):
                packetList.insert(
                    dataLen + 8 + i, self.__cvt_value2byte(packetCRC32, i + 1, 1)
                )

            packetList.insert(0, len(packetList))

            hex_list = [hex(item) for item in packetList]
            return packetList

    def __decode_btl_packet(self, waitingMsg=None):
        rxdata = self.__read_from_serial_until(DataLength=2)
        ack_byte = rxdata[0]
        length_if_data = rxdata[1] if len(rxdata) > 1 else 0

        if ack_byte == 1:
            print("\nReceived ACK from bootloader")

            if length_if_data > 0:
                data_bytes = self.__read_from_serial_until(
                    DataLength=length_if_data, WaitingMsg=waitingMsg
                )

                # Convert received bytes to a list of hexadecimal strings
                hex_values = [format(byte, "02X") for byte in data_bytes]
                ascii_string = data_bytes.decode("ascii", errors="replace")

                print("\nReceived from bootloader:")
                print(f"\tBYTES: {hex_values}")
                print(f"\tASCII: {ascii_string}")

                return 1
            else:
                print("\nNothing to wait from bootloader")
                return 2
        else:
            hex_values = [format(ack_byte, "02X"), format(length_if_data, "02X")]
            print(f"\nReceived Nack from bootloader err_code@{hex_values[1]}")
            return 0

    def __btl_cmd_eraseFlash(self, pageIdx: int, pageN: int):
        data = [pageIdx, pageN]
        packet_info = {
            "PacketType": 1,
            "Command": btl_ttl_intf.BtlCommands.CBL_FLASH_ERASE_CMD.value,
            "Data": data,
        }  # Write to address
        packet = self.__encode_btl_packet(packet_info)
        print(f"Packet sent ({packet[0]}):")
        self.__write_to_serial_until(packet[0], 1)
        time.sleep(0.5)
        for Data in packet[1:]:
            self.__write_to_serial_until(Data, len(packet) - 1)
        if 0 == self.__decode_btl_packet("Waiting reply from bootloader"):
            print("Failed to erase application")
            sys.exit(1)

    #! TODO(Wx): Optimize the flashing algorithm for speed
    def __btl_cmd_flashApp(self):
        start_time = time.monotonic()
        START_ADDR = 0x08008000
        self.__btl_cmd_eraseFlash(30, 60)
        File_Total_Len = os.path.getsize(self.__application_bin_fp)
        bin_file = open(self.__application_bin_fp, "rb")
        data = []
        Addr = START_ADDR
        BinFileSentBytes = 0
        BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
        while BinFileRemainingBytes:
            """ Read 128 bytes from the binary file each time """
            if BinFileRemainingBytes >= btl_ttl_intf.MAX_DATA_PER_PACKET_LENGTH:
                BinFileReadLength = btl_ttl_intf.MAX_DATA_PER_PACKET_LENGTH
            else:
                BinFileReadLength = BinFileRemainingBytes

            bindata = []
            bindata = bin_file.read(btl_ttl_intf.MAX_DATA_PER_PACKET_LENGTH)

            data = [
                self.__cvt_value2byte(Addr, 1, 1),
                self.__cvt_value2byte(Addr, 2, 1),
                self.__cvt_value2byte(Addr, 3, 1),
                self.__cvt_value2byte(Addr, 4, 1),
                BinFileReadLength,
            ] + list(bindata)

            packet_info = {
                "PacketType": 1,
                "Command": btl_ttl_intf.BtlCommands.CBL_MEM_WRITE_CMD.value,  # Write to address
                "Data": data,
            }
            packet = self.__encode_btl_packet(packet_info)
            print(f"Packet sent ({packet[0]}):")
            self.__write_to_serial_until(packet[0], 1)

            # time.sleep(1)
            for Data in packet[1:]:
                self.__write_to_serial_until(Data, len(packet) - 1)

            # time.sleep(2)  # wait for writing
            if 0 == self.__decode_btl_packet("Waiting a reply from bootloader"):
                print("Failed to program bootloader")
                sys.exit(1)

            Addr += BinFileReadLength
            BinFileSentBytes += BinFileReadLength
            BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
            print("\n   Bytes sent to the bootloader :{0}".format(BinFileSentBytes))
        end_time = time.monotonic()
        print(f"Flashing done.")
        print(f"Time taken to flash application: {end_time - start_time}")

    def __btl_cmd_jumpToAddr(self, applicationAddress=None):
        assert applicationAddress is not None, sys.exit(-1)

        data = list(struct.pack("<I", applicationAddress))

        packet_info = {
            "PacketType": 1,
            "Command": btl_ttl_intf.BtlCommands.CBL_GO_TO_ADDR_CMD.value,
            "Data": data,
        }
        packet = self.__encode_btl_packet(packet_info)
        time.sleep(2)
        print("Packet sent:")
        self.__write_to_serial_until(packet[0], 1)
        time.sleep(0.5)
        for Data in packet[1:]:
            self.__write_to_serial_until(Data, len(packet) - 1)

        if 0 == self.__decode_btl_packet("Nothing to wait from bootloader"):
            print("Failed to jump to application")
            sys.exit(1)

    def __btl_cmd_swReboot(self):
        data = []
        packet_info = {
            "PacketType": 1,
            "Command": btl_ttl_intf.BtlCommands.CBL_SW_RESET.value,
            "Data": data,
        }
        packet = self.__encode_btl_packet(packet_info)
        print("Packet sent:")
        self.__write_to_serial_until(packet[0], 1)
        for Data in packet[1:]:
            self.__write_to_serial_until(Data, len(packet) - 1)

        if 0 == self.__decode_btl_packet("Nothing to wait from bootloader"):
            print("Failed to reset app")
            sys.exit(1)

    @staticmethod
    def __calculate_crc32_04C11DB7(Buffer, BufferLength):
        CRC_Value = 0xFFFFFFFF
        for DataElem in Buffer[0:BufferLength]:
            CRC_Value = CRC_Value ^ DataElem
            for DataElemBitLen in range(32):
                if CRC_Value & 0x80000000:
                    CRC_Value = (CRC_Value << 1) ^ 0x04C11DB7
                else:
                    CRC_Value = CRC_Value << 1
        return CRC_Value & 0xFFFFFFFF

    @staticmethod
    def __cvt_value2byte(WordValue, ByteIdx, Byte_Lower_First):
        Byte_Value = WordValue >> (8 * (ByteIdx - 1)) & 0x000000FF
        return Byte_Value

    def __cvtHex2Bin(self, hex_fp):
        output = os.path.splitext(os.path.basename(hex_fp))[0]
        
        # Load the Intel HEX file
        ih = IntelHex(hex_fp)

        # Convert to binary and save
        ih.tobinfile(OUTPUT_BIN_FILE_STD, '/', output,'.bin')
    
    def __print_commands_as_table(self):
        print(
            "+-------+---------------------------+----------------------------------------------------+"
        )
        print("| {:<5} | {:<25} | {:<50} |".format("Index", "Command", "Description"))
        print(
            "+-------+---------------------------+----------------------------------------------------+"
        )

        for i, (command, description) in enumerate(
            self.__btl_available_commands.items(), start=1
        ):
            print("| {:<5} | {:<25} | {:<50} |".format(i, command, description))
            print(
                "+-------+---------------------------+----------------------------------------------------+"
            )

    def __print_header(self):
        print(
            "+----------------------------------------------------------------------------------------+"
        )
        print(
            "|            SECURE AND FAST FOTA UPDATE FOR AUTOMOTIVE BOOTLOADER                       |"
        )
        print(
            "+----------------------------------------------------------------------------------------+"
        )
        print(
            "|                  - Mohamed Ashraf  Wx                                                  |"
        )
        print(
            "+----------------------------------------------------------------------------------------+"
        )
    
    def check_input(self, input_str):
        try:
            # Check if input is empty or contains only whitespaces
            if not input_str or input_str.isspace():
                pass
            
            if len(input_str) == 0:
                return False
            
            # Remove leading and trailing whitespaces
            input_str = input_str.strip()
            
            # Convert input to integer
            input_value = int(input_str)

            # All checks passed, return the sanitized input
            return input_value
        except ValueError as e:
            print(e)
            return None
    
    def __interactive_btl_intf(self):
        print("Select a command")
        selected_command = input("[BTL]> ")
        selected_command = self.check_input(selected_command)
        if False == selected_command:
            return False
        self.btl_command_exec(selected_command - 1)

    def BTL_CLI(self):
        try:
            while True:
                self.__print_header()
                self.__print_commands_as_table()
                self.__interactive_btl_intf()
        except KeyboardInterrupt:
            print("\n\n\tCopyrights (c) 2023, Mohamed Ashaf Wx ")

    def run(self):
        # Get the user input
        # self.set_serial_port(input("Enter the serial port to connect to: "))
        # self.set_baudrate(input("Enter the baudrate: "))
        # self.set_timeout(input("Enter the timeout: "))
        self.set_serial_port('COM3')
        self.set_baudrate(115200)

        if self.__connect_to_port(self.__serial_port, self.__baudrate, self.__timeout):
            self.BTL_CLI()

    # APIs
    def get_serial_port(self):
        return self.__serial_port

    def set_serial_port(self, value):
        self.__serial_port = value

    def get_baudrate(self):
        return self.__baudrate

    def set_baudrate(self, value):
        if int(value) in btl_ttl_intf.std_baudrates:
            self.__baudrate = value
        else:
            logging.error("Invalid baudrate specified, list of available baudrates:")
            for i in range(len(btl_ttl_intf.std_baudrates)):
                logging.info(btl_ttl_intf.std_baudrates[i])
            raise ValueError("Invalid baudrte value")

    def get_timeout(self):
        return self.__timeout

    def set_timeout(self, value):
        self.__timeout = int(value)

    def set_appBinFp(self, value):
        if os.path.exists(value):
            self.__application_bin_fp = value
        else:
            logging.error("Invalid application binary path")

    def cvtHex2Bin(self, hexfp):
        if os.path.exists(hexfp):
            self.__cvtHex2Bin(self, hexfp)
        else:
            print(f"Invalid file path @{hexfp}")
    
    def btl_cmd_intf_JumpToAddr(self):
        jumpAddr = int(
            input("Input flashing address in hex format (e.x 0x08008000): "), base=16
        )
        self.__btl_cmd_jumpToAddr(jumpAddr)

    def btl_cmd_intf_EraseFlash(self):
        pageIdx = int(input("Input the page idx (0-63): "), base=10)
        pageN = int(input("Input number of pages to erase: "), base=10)

        self.__btl_cmd_eraseFlash(pageIdx, pageN)

    def btl_cmd_intf_FlashApp(self):
        self.__btl_cmd_flashApp()

    def btl_cmd_intf_Reboot(self):
        self.__btl_cmd_swReboot()

    def btl_command_exec(self, command):
        default_command = lambda: print("Invalid command")
        command_method = self.__btl_commands.get(command, default_command)
        command_method()
    
if __name__ == "__main__":
    mybtl = btl_ttl_intf(True, False)
    mybtl.run()
