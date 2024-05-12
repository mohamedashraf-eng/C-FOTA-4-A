import logging
import yaml
import json
import os
from os import urandom
import argparse
#
from Crypto.Cipher import AES
from Crypto.Hash import CMAC, HMAC, SHA256, SHA512
from Crypto.Protocol.KDF import PBKDF2
import hashlib
from intelhex import IntelHex

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
        self.__hmac_secret_key = b"mohamedAshraf"
        self.__cmac_secret_key = b"mohamedAshraf"
        self.__cmac_salt = b"mohamedashraf"
        self.__ecc_salt = b"mohamedashraf"

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
        # output = os.path.splitext(os.path.basename(hex_fp))[0]

        # Load the Intel HEX file
        ih = IntelHex(self.__firmware_fp)

        # Convert to binary and save
        ih.tobinfile("UpdatedFirmware.bin")
        self.__firmware_fp_bin = "./UpdatedFirmware.bin"
    # def __calculate_sha256(self):
    #     sha256_obj = SHA256.new()
    #     #sha256_hash = hashlib.sha256(self.__firmware_in_bin).digest()
    #     for line in self.__firmware_hex:
    #         sha256_obj.update(str(line).encode('utf-8'))

    #     return sha256_obj.digest()
    
    def __calculate_sha256(self):
        sha256_hash = hashlib.sha256()
        with open(self.__firmware_fp_bin, "rb") as f:
            # Read the file in chunks of 4096 bytes
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return str(sha256_hash.hexdigest())

    def __calculate_cmac(self):
        sha256_hash = self.__calculate_sha256()
        cmac_key = PBKDF2(self.__cmac_secret_key, self.__cmac_salt, dkLen=16)
        cmac_cipher = CMAC.new(cmac_key, ciphermod=AES)
        cmac_cipher.update(sha256_hash)
        cmac_digest = cmac_cipher.hexdigest()
        return cmac_digest

    def __calculate_hmac(self):
        hmac_obj = HMAC.new(str(self.__hmac_secret_key).encode(
            'utf-8'), digestmod=SHA256)
        hmac_obj.update(str(self.__calculate_sha256()).encode('utf-8'))
        return hmac_obj.digest()

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

    def __firmware_encrypt_ecc(self):
        pass

    def __firmware_dsa(self):
        pass
    #
    #

    def get_sha256(self):
        return self.__calculate_sha256()

    def get_cmac(self):
        return self.__calculate_cmac()

    def get_hmac(self):
        return self.__calculate_hmac()

    def enc_firmware(self):
        return self.__firmware_encrypt_ecc()

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
    print(f'SHA256-Hash: {myFirmware.get_sha256()}')
    # print(f'CMAC Of Hash: {myFirmware.get_cmac()}')
    # print(f'HMAC Of Hash: {myFirmware.get_hmac().hex()}')
