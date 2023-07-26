import logging
import yaml
import json
import os
import argparse
import hashlib
import hmac
from Crypto.Cipher import AES
from Crypto.Hash import CMAC
from Crypto.Protocol.KDF import PBKDF2

# Configure logging
log_filename = "main.log"
log_format = (
    "%(asctime)s - %(levelname)s - %(message)s - %(pathname)s:%(lineno)d - %(funcName)s"
)
logging.basicConfig(filename=log_filename,
                    level=logging.INFO, format=log_format)


class prj_foem_firmware:
    def __init__(self, firmware_fp):
        self.__firmware_fp = firmware_fp
        self.__firmware_hex = []
        self.__firmware_in_json = None
        self.__firmware_in_bin = None
        self.__firmware_size = None
        self.__hmac_secret_key = b"mohamedashraf&abdolotfy&drsalma"
        self.__cmac_secret_key = b"mohamedashraf&abdolotfy&drsalma"
        self.__cmac_salt = b"mohamedashraf"

        self.__firmware_hex_fetch()
        self.__firmware_in_json = json.dumps(self.__firmware_hex)
        self.__firmware_cvt2bin()

    def __firmware_hex_fetch(self):
        try:
            with open(self.__firmware_fp, "r") as hex_file:
                self.__firmware_hex.append(hex_file.readlines())
            logging.info(f"Fetched the hex file successfully.")
        except FileNotFoundError:
            logging.error(f"Error: File '{self.__firmware_fp}' not found.")

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
            self.__firmware_in_bin = binary_data
        except FileNotFoundError:
            logging.error("Error: The specified hex file was not found.")

    def __calculate_sha256(self):
        sha256_hash = hashlib.sha256(self.__firmware_in_bin).hexdigest()
        return sha256_hash

    def __calculate_cmac(self):
        cmac_key = PBKDF2(self.__cmac_secret_key, self.__cmac_salt, dkLen=16)
        cmac_cipher = CMAC.new(cmac_key, ciphermod=AES)
        cmac_cipher.update(self.__firmware_in_bin)
        cmac_digest = cmac_cipher.digest()
        return cmac_digest

    def __calculate_hmac(self):
        hmac_obj = hmac.new(
            self.__hmac_secret_key, self.__firmware_in_bin, hashlib.sha256
        )
        hmac_digest = hmac_obj.hexdigest()
        return hmac_digest

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

                return firmware_size
            logging.info(f"Calculated firmware size successfully.")
        except FileNotFoundError:
            logging.error(f"Error: File '{self.__firmware_fp}' not found.")
            return None

    def get_sha256(self):
        return self.__calculate_sha256()

    def get_cmac(self):
        return self.__calculate_cmac()

    def get_hmac(self):
        return self.__calculate_hmac()

    def get_size(self):
        return self.__firmware_calculate_size()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-fp', '--file_path', type=str,
                        help='Specify the file path')
    args = parser.parse_args()
    file_path = args.file_path

    myFirmware = prj_foem_firmware(file_path)
    print(f'size: {myFirmware.get_size()}')
    print(f'CMAC: {myFirmware.get_cmac()}')
    print(f'HMAC: {myFirmware.get_hmac()}')
    print(f'SHA256: {myFirmware.get_sha256()}')
