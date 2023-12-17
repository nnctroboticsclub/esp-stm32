#!/bin/bash

SERVER=$1
FILE=$2

if [ -z "$SERVER" ]; then
    echo "Usage: $0 <server> <file>"
    exit 1
fi

if [ -z "$FILE" ]; then
    echo "Usage: $0 <server> <file>"
    exit 1
fi

if [ ! -f "$FILE" ]; then
    echo "File $FILE does not exist"
    exit 1
fi

printf "\n\e[33mBooting Bootloader\n"
curl -X POST http://${SERVER}/api/stm32/bootloader/boot

printf "\n\e[32mUploading $FILE to $SERVER\n"
curl --data-binary @$FILE http://${SERVER}/api/stm32/bootloader/upload

printf "\n\e[33mResetting MCU...\n"
curl -X POST http://${SERVER}/api/stm32/reset

printf "\n\e[32mFlashing Done!\n"