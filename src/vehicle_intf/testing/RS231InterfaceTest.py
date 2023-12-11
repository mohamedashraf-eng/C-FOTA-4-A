import serial
import struct
import binascii
from time import sleep
import sys
import os

verbose_mode = 1
Memory_Write_Active = 0

def Check_Serial_Ports():
    Serial_Ports = []

    if sys.platform.startswith('win'):
        Ports = ['COM%s' % (i + 1) for i in range(256)]
    else:
        raise EnvironmentError("Error !! Unsupported Platform \n")

    for Serial_Port in Ports:
        try:
            test = serial.Serial(Serial_Port)
            test.close()
            Serial_Ports.append(Serial_Port)
        except (OSError, serial.SerialException):
            pass

    return Serial_Ports

def Serial_Port_Configuration(Port_Number, br):
    global Serial_Port_Obj
    try:
        Serial_Port_Obj = serial.Serial(Port_Number, br, timeout=1)
    except:
        print("\nError !! That was not a valid port")

        Port_Number = Check_Serial_Ports()
        if(not Port_Number):
            print("\nError !! No ports Detected")
        else:
            print("\nHere are some available ports on your PC. Try Again !!")
            print("\n   ", Port_Number)
        return -1

    if Serial_Port_Obj.is_open:
        print("Port Open Success \n")
    else:
        print("Port Open Failed \n")


def Write_Data_To_Serial_Port(Value, Length):
    _data = struct.pack('>B', Value)
    if(verbose_mode):
        Value = bytearray(_data)
        print("   " + "0x{:02x}".format(Value[0]), end=' ')
        if(Memory_Write_Active and (not verbose_mode)):
            print("#", end=' ')
        Serial_Port_Obj.write(_data)


def Read_Serial_Port(Data_Len):
    Serial_Value = Serial_Port_Obj.read(Data_Len)
    Serial_Value_len = len(Serial_Value)
    while Serial_Value_len <= 0:
        Serial_Value = Serial_Port_Obj.read(Data_Len)
        Serial_Value_len = len(Serial_Value)
        print("Waiting Reply from the Bootloader")
    return Serial_Value


def Calculate_CRC32(Buffer, Buffer_Length):
    CRC_Value = 0xFFFFFFFF
    for DataElem in Buffer[0:Buffer_Length]:
        CRC_Value = CRC_Value ^ DataElem
        for DataElemBitLen in range(32):
            if CRC_Value & 0x80000000:
                CRC_Value = (CRC_Value << 1) ^ 0x04C11DB7
            else:
                CRC_Value = CRC_Value << 1
    return CRC_Value & 0xFFFFFFFF

def Word_Value_To_Byte_Value(Word_Value, Byte_Index, Byte_Lower_First):
    Byte_Value = (Word_Value >> (8 * (Byte_Index - 1)) & 0x000000FF)
    return Byte_Value

"""
    Testing packet:
    [Packet Length] [Packet Type] [Command] [Data Length] [Data]    [Data CRC32] [Packet CRC32]
    [1 Byte]        [1 Byte]      [1 Bytte] [1 Byte]      [n Bytes] [4 Bytes]    [4 Bytes]
"""
def formatPacket(packetDict: dict=None):
    if packetDict is not None:
        packetList = []
        #
        PacketType = packetDict['PacketType']
        Command = packetDict['Command']
        #
        data = packetDict['Data']
        dataLen = len(data)
        dataCRC32 = Calculate_CRC32(data, dataLen)
        #
        packetList.append(PacketType)
        packetList.append(Command)
        packetList.append(dataLen)
        packetList = packetList[:5] + data + packetList[5:]
        
        # Calculate Data CRC32 and insert into packetList
        for i in range(4):
            packetList.insert(dataLen + 4 + i, Word_Value_To_Byte_Value(dataCRC32, i + 1, 1))

        # Calculate Packet CRC32 and insert into packetList
        packetCRC32 = Calculate_CRC32(packetList, len(packetList))
        for i in range(4):
            packetList.insert(dataLen + 8 + i, Word_Value_To_Byte_Value(packetCRC32, i + 1, 1))

        packetList.insert(0, len(packetList))
        
        hex_list = [hex(item) for item in packetList]
        # print(f"Packet: {hex_list}")
        # print(f"[Packet Length][Packet Type][Command][Data Length][Data][Data CRC32][Packet CRC32]")
        return packetList
    
def readIncomingMessageFromBl():
    print('\n')
    reply = Read_Serial_Port(2) 
    if reply[0] == 1:
        print("\nReceived Ack from bootloader")
        if reply[1] > 0:
            reply = Read_Serial_Port(reply[1])
            # Convert received bytes to a list of hexadecimal strings
            hex_values = [format(byte, '02X') for byte in reply]
            ascii_string = reply.decode('ascii', errors='replace')
            print(f"\nReceived from bootloader: ")
            print(f"\tBYTES: {hex_values}")
            print(f"\tASCII: {ascii_string}")
            return 1
        else:
            print("\nNothing to wait from bootloader")
            return 2
    else:
        hex_values = [format(byte, '02X') for byte in reply]
        print(f"\nReceived Nack from bootloader err_code@{hex_values[1]}")
        return 0


def parse_hex(hex_fp):
    bin_data = []

    try:
        with open(hex_fp, 'r') as f:
            hex_lines = f.readlines()

        for line in hex_lines:
            if line.startswith(':10'):
                data_hex = line[10:-2].strip()
                bin_data.extend(bytes.fromhex(data_hex))

        return bin_data

    except FileNotFoundError:
        print("File not found.")
        return None
    except Exception as e:
        print("An error occurred:", str(e))
        return None
    
START_ADDR = 0x08008000
 
def EraseApplication():
    data = [30, 60]
    packet_info = {
        "PacketType": 1,
        "Command": 5,  # Write to address
        "Data": data
    }
    packet = formatPacket(packet_info)
    print("Packet sent:")
    Write_Data_To_Serial_Port(packet[0], 1)
    sleep(0.5)
    for Data in packet[1:]:
        Write_Data_To_Serial_Port(Data, len(packet) - 1)
    if 0 == readIncomingMessageFromBl():
        print("Failed to erase application")
        sys.exit(1)

DATA_PER_PACKET = 100

def FlashApplication(application_fp):
    # bin_data = parse_hex(r"G:\WX_CAREER\Grad Project\src\oem_intf\django_webappv1\fota\firmwares\bootloaderX3.hex")

    EraseApplication()

    File_Total_Len = os.path.getsize(application_fp)
    bin_file = open(application_fp, 'rb')
    data = []
    Addr = START_ADDR
    BinFileSentBytes = 0
    BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
    while BinFileRemainingBytes:
        ''' Read 128 bytes from the binary file each time '''
        if(BinFileRemainingBytes >= DATA_PER_PACKET):
            BinFileReadLength = DATA_PER_PACKET
        else:
            BinFileReadLength = BinFileRemainingBytes
        
        bindata = []
        bindata = bin_file.read(DATA_PER_PACKET)
        
        data = [Word_Value_To_Byte_Value(Addr, 1, 1), Word_Value_To_Byte_Value(Addr, 2, 1),
                Word_Value_To_Byte_Value(Addr, 3, 1), Word_Value_To_Byte_Value(Addr, 4, 1), BinFileReadLength] + list(bindata)
        
        packet_info = {
            "PacketType": 1,
            "Command": 6,  # Write to address
            "Data": data
        }
        packet = formatPacket(packet_info)
        print(f"Packet sent ({packet[0]}): ")
        Write_Data_To_Serial_Port(packet[0], 1)

        sleep(1)
        for Data in packet[1:]:
            Write_Data_To_Serial_Port(Data, len(packet) - 1)

        # sleep(2)  # wait for writing
        if 0 == readIncomingMessageFromBl():
            print("Failed to program bootloader")
            sys.exit(1)

        Addr += BinFileReadLength
        BinFileSentBytes += BinFileReadLength
        BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
        print("\n   Bytes sent to the bootloader :{0}".format(
            BinFileSentBytes))
        
if __name__ == "__main__":
    serial_port = Serial_Port_Configuration('COM3', 115200)
    
    import time
    start_time = time.monotonic()
    FlashApplication(r"G:\WX_CAREER\Grad Project\src\vehicle_intf\testing\TestAppLedOn\MDK-ARM\TestAppLedOn\TestAppLedOn.bin")
    end_time = time.monotonic()
    elapsed_time = end_time - start_time
    print(f"Firmware flashing time: {elapsed_time:.3f}s")
    
    # while True: pass
    
    # # Jumping to the flashed application [TEST]
    # # 0x08008235 ??
    # data = [0x00, 0x80, 0x00, 0x08]
    # packet_info = {
    #     "PacketType": 1,
    #     "Command": 4,
    #     "Data": data,
    # }
    # packet = formatPacket(packet_info)

    # sleep(2)
    # print("Packet sent:")
    # Write_Data_To_Serial_Port(packet[0], 1)
    # sleep(0.5)
    # for Data in packet[1:]:
    #     Write_Data_To_Serial_Port(Data, len(packet) - 1)
    # readIncomingMessageFromBl()