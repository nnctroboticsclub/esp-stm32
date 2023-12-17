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

printf "\e[33mBooting Bootloader\n"
curl -X POST http://${SERVER}/api/stm32/bootloader/boot
printf "\e[32mUploading $FILE to $SERVER\n"
curl -X POST -d $FILE http://${SERVER}/api/stm32/bootloader/upload