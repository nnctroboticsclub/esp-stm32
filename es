#!/bin/fish

set PWSH /mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe

set STM32 $HOME/Documents/PlatformIO/Projects/NW-R
set ARDUINO $HOME/Documents/PlatformIO/Projects/Arduino-I2CSlave-Test

set SELF (status filename)

set task $argv[1]
if test -z $task
  echo "Usage: $0 <task>"
  exit 1
end

switch $task
case "count_lines"
  eval $(echo wc -lc $(find . -regextype posix-egrep -regex '.\/(components|main)/.*\.[hc](pp)?' | sed "/*bin.h*/d")) | sort -rn
case "stm32-flash"
  cd $STM32
  pio run -t upload
case "update-data-proxy"
  begin
   git -C ~/ghq/github.com/nnctroboticsclub/esp-stm32/components/data-proxy checkout .
   git -C ~/ghq/github.com/nnctroboticsclub/esp-stm32/components/data-proxy pull
  end &
  begin
   git -C ~/Documents/PlatformIO/Projects/NW-R/third-party/data-proxy checkout .
   git -C ~/Documents/PlatformIO/Projects/NW-R/third-party/data-proxy pull
  end &
  wait
  printf "\e[32mDone!\e[m\n"
case "attach-usb-hub"
  $PWSH usbipd wsl attach -b 5-1
  $PWSH usbipd wsl attach -b 5-2
  $PWSH usbipd wsl attach -b 5-3
case "attach-usb-pc"
  $PWSH usbipd wsl attach -b 1-1
  $PWSH usbipd wsl attach -b 1-2
case "monitor"
  printf "\x1b[33mSetuping serial...\x1b[m\n"
  stty -F /dev/ttyACM0 9600 raw -echo
  stty -F /dev/ttyUSB0 115200 raw -echo
  stty -F /dev/ttyACM1 9600 raw -echo
  sleep 0.5

  printf "\x1b[1;31mInitialized!\x1b[m\n"

  begin
    # STM32 Monitor
    cat /dev/ttyACM1 | sed -u 's/^/\x1b[31m[STM32]\x1b[m /'
  end &

  begin
    # Arduino Monitor
    cat /dev/ttyACM0 | sed -u 's/^/\x1b[33m[ AVR ]\x1b[m /'
  end &

  wait

  kill $(jobs -p)
case "s"
  complete -c es -x
  complete -c es -f -n '__fish_use_subcommand' -d 'Count code base lines' -a 'count_lines'
  complete -c es -f -n '__fish_use_subcommand' -d 'Update Data Proxy in 2 repositories' -a 'update-data-proxy'
  complete -c es -f -n '__fish_use_subcommand' -d 'Attach USB (USB Hub)' -a 'attach-usb-hub'
  complete -c es -f -n '__fish_use_subcommand' -d 'Start AVR/STM32 Monitor' -a 'monitor'
  complete -c es -f -n '__fish_use_subcommand' -d 'Do fish complete' -a 's'
  complete -c es -f -n '__fish_use_subcommand' -d 'Flash STM32' -a 'stm32-flash'

  if test "$ES_RUNNER_LOADED_ENVIRONMENT" != "1"
    printf "\x1b[33mLoading environment...\x1b[m\n"
    source ~/.platformio/penv/bin/activate.fish > /dev/null
    source ~/.platformio/packages/framework-espidf/export.fish > /dev/null
    set -ax PATH (dirname $SELF)

    set -x ES_RUNNER_LOADED_ENVIRONMENT 1
  end
end
