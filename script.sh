#!/bin/bash

# This script subscribes to a MQTT topic using mosquitto_sub.
# On each message received, you can execute whatever you want.

while true  # Keep an infinite loop to reconnect when connection lost/broker unavailable
do
    mosquitto_sub -h "192.168.1.63" -t "esp32/password" | while read -r payload
    do
    echo ${payload}
    # Here is the callback to execute whenever you receive a message:
    if [[ ${payload} == "1" ]]
    then
        echo "default"
        mosquitto_pub -t "esp32/LEDS/1" -h "192.168.1.63" -m 1
    fi
    if [[ ${payload} == "11" ]]
    then
        echo "blue"
        mosquitto_pub -t "esp32/LEDS/2" -h "192.168.1.63" -m 1
    fi
    if [[ ${payload} == "21" ]]
    then
        echo "red"
        mosquitto_pub -t "esp32/LEDS/2" -h "192.168.1.63" -m 0
    fi
    done
    sleep 5  # Wait 10 seconds until reconnection
done # &  # Discomment the & to run in background (but you should rather run THIS script in background)