# Use the official Eclipse Mosquitto image as the base
FROM eclipse-mosquitto:latest

# Copy the configuration files into the container
COPY broker.conf /mosquitto/config/mosquitto.conf
COPY dynamic-security.json /mosquitto/config/dynamic-security.json

# Expose port 1883 for MQTT
EXPOSE 1883
EXPOSE 9001

# TO BUILD:
# > docker build -t mosquitto-broker-greenhouse-iot .

# TO RUN:
# docker run -d --name mosquitto-broker-greenhouse-iot -p 1883:1883 -p 9001:9001 -v ${PWD}/mosquitto/config:/mosquitto/config -v ${PWD}/mosquitto/data:/mosquitto/data -v ${PWD}/mosquitto/log:/mosquitto/log mosquitto-broker-greenhouse-iot