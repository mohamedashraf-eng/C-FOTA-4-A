@echo off
setlocal enabledelayedexpansion

REM Set the directory
set "DIR=mqtt_certs"

REM Create CA Folder if it doesn't exist
if not exist %DIR%\ca mkdir %DIR%\ca

REM Generate CA Certificate and Key with passphrase "1332" using SHA-3 (512-bit) and Ed25519
openssl req -new -x509 -days 365 -extensions v3_ca -keyout %DIR%\ca\ca.key -out %DIR%\ca\ca.crt -subj "/CN=MyCA" -passout pass:1332 -newkey ed25519

REM Create Broker Folder if it doesn't exist
if not exist %DIR%\broker mkdir %DIR%\broker

REM Generate Broker Certificate and Key with passphrase "1332" using SHA-3 (512-bit) and Ed25519
openssl genpkey -algorithm Ed25519 -out %DIR%\broker\broker.key -aes256 -pass pass:1332
openssl req -out %DIR%\broker\broker.csr -key %DIR%\broker\broker.key -new -subj "/CN=MyBroker" -newkey ed25519 -passin pass:1332
openssl x509 -req -in %DIR%\broker\broker.csr -CA %DIR%\ca\ca.crt -CAkey %DIR%\ca\ca.key -CAcreateserial -out %DIR%\broker\broker.crt -days 365 -passin pass:1332
del %DIR%\broker\broker.csr

REM Create Client Folder if it doesn't exist
if not exist %DIR%\client mkdir %DIR%\client

REM Set the number of clients from the environment argument or default to 1
set "NUM_CLIENTS=%1"
if not defined NUM_CLIENTS set "NUM_CLIENTS=1"

REM Generate Client Certificates and Keys with passphrase "1332" using SHA-3 (512-bit) and Ed25519
for /L %%i in (1, 1, %NUM_CLIENTS%) do (
    openssl genpkey -algorithm Ed25519 -out %DIR%\client\client%%i.key -aes256 -pass pass:1332
    openssl req -out %DIR%\client\client%%i.csr -key %DIR%\client\client%%i.key -new -subj "/CN=MyClient%%i" -newkey ed25519 -passin pass:1332
    openssl x509 -req -in %DIR%\client\client%%i.csr -CA %DIR%\ca\ca.crt -CAkey %DIR%\ca\ca.key -CAcreateserial -out %DIR%\client\client%%i.crt -days 365 -passin pass:1332
    del %DIR%\client\client%%i.csr
)

@REM REM Append certificate paths and configurations to mosquitto.conf
@REM (
@REM     echo listener 8883
@REM     echo # Path to the PEM encoded server certificate.
@REM     echo certfile %CD%\%DIR%\broker\broker.crt
@REM     echo.
@REM     echo # Path to the PEM encoded keyfile.
@REM     echo keyfile %CD%\%DIR%\broker\broker.key
@REM     echo.
@REM     echo cafile %CD%\%DIR%\ca\ca.crt
@REM     echo.
@REM     echo use_identity_as_username true
@REM     echo require_certificate true
@REM     echo allow_anonymous false
@REM     echo protocol mqtt
@REM ) >> "F:\mosquitto\mosquitto.conf"

echo Certificates and folders created successfully.
