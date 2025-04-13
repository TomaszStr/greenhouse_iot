package com.greenhouse.greenhouse_iot.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.greenhouse.greenhouse_iot.model.dto.MqttCredentials;
import com.greenhouse.greenhouse_iot.model.mqtt.alert.MqttSensorAlert;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_command.MqttSensorCommandType;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.CommandsWrapper;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttAcl;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_reading.SensorReadingMqtt;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import com.greenhouse.greenhouse_iot.model.enums.SensorState;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_command.MqttSensorCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command.CreateClientCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command.CreateRoleCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttRole;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command.MqttSetClientPassword;
import com.greenhouse.greenhouse_iot.utils.PasswordGenerator;
import jakarta.annotation.PostConstruct;
import lombok.extern.slf4j.Slf4j;
import org.eclipse.paho.client.mqttv3.*;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

@Slf4j
@Service
public class MqttService {

    private final MqttClient mqttClient;
    private final ObjectMapper objectMapper;
    private final SensorReadingService sensorReadingService;
    private final PasswordGenerator passwordGenerator;
    private final Set<String> subscribedTopics;
    private final String brokerUrl;
    private final String clientId;
    private final String mqttUsername;
    private final String mqttPassword;
    private final String mqttDynamicSecurityTopic;

    public MqttService(
            @Value("${mqtt.broker-url}") String brokerUrl,
            @Value("${mqtt.client-id}") String clientId,
            @Value("${mqtt.username}") String mqttUsername,
            @Value("${mqtt.password}") String mqttPassword,
            @Value("${mqtt.dynamic-security-topic}") String mqttDynamicSecurityTopic,
            ObjectMapper objectMapper,
            SensorReadingService sensorReadingService,
            PasswordGenerator passwordGenerator) throws MqttException {

        log.info("Mqtt config data: broker-url:{}, client-id:{}, username:{}", brokerUrl, clientId, mqttUsername);
        this.brokerUrl = brokerUrl;
        this.clientId = clientId;
        this.mqttUsername = mqttUsername;
        this.mqttPassword = mqttPassword;
        this.mqttDynamicSecurityTopic = mqttDynamicSecurityTopic;

        mqttClient = new MqttClient(brokerUrl, clientId);
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true);
        options.setAutomaticReconnect(true);
        options.setUserName(mqttUsername);
        options.setPassword(mqttPassword.toCharArray());
        mqttClient.connect(options);

        this.passwordGenerator = passwordGenerator;
        this.objectMapper = objectMapper;
        this.sensorReadingService = sensorReadingService;
        subscribedTopics = new HashSet<>();

        mqttClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                log.info("MQTT connected,  reconnect: {},  sever URI:{}", reconnect, serverURI);
                try {
                    for(var topic : subscribedTopics) {
                        log.info("Resubscribe to topic after reconnect: {}", topic);
                        mqttClient.subscribe(topic);
                    }
                } catch (MqttException e) {
                    throw new RuntimeException(e);
                }
            }

            @Override
            public void connectionLost(Throwable cause) {
                log.error("MQTT connection lost, cause: {}", cause.getMessage());
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) {
                log.info("Message received on topic {}: {}", topic, new String(message.getPayload()));
                if (topic.matches("devices/[^/]+/data")) {
                    log.info("Handling data topic for device.");
                    handleSensorMqttReading(topic, message);
                } else if (topic.matches("devices/[^/]+/alerts")) {
                    log.info("Handling alerts topic for device.");
                    handleSensorMqttAlert(topic, message);
                } else {
                    log.error("Unknown topic: {}", topic);
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                try {
                    log.info("Delivery complete: {}",token.getMessage().toString());
                } catch (MqttException e) {
                    throw new RuntimeException(e);
                }
            }
        });
    }

    @PostConstruct
    public void initialize() throws MqttException {
        subscribeToTopic("devices/+/data", 0);
        subscribeToTopic("devices/+/alerts", 0);
        subscribeToTopic(this.mqttDynamicSecurityTopic, 0);
    }

    private void handleSensorMqttReading(String topic, MqttMessage message) {
        SensorReadingMqtt sensorReadingMqtt = null;
        try {
            log.info("Map json to reading");
            sensorReadingMqtt = objectMapper.readValue(new String(message.getPayload()), SensorReadingMqtt.class);
            sensorReadingMqtt.setSensorMqttName(getSensorNameFromTopic(topic));

            log.info("Save reading to DB: {}", sensorReadingMqtt);
            if(sensorReadingService.addMqttReading(sensorReadingMqtt)) {
                log.info("Mqtt reading saved successfully to DB");
            }
            else {
                log.info("Mqtt reading saved unsuccessfully to DB");
            }
        } catch (JsonProcessingException | RuntimeException e) {
            log.error("Exception while handling new message: {}",e.getMessage());
            throw new RuntimeException(e);
        }
    }

    private void handleSensorMqttAlert(String topic, MqttMessage message) {
        MqttSensorAlert mqttSensorAlert = null;
        try {
            log.info("Map json to alert");
            mqttSensorAlert = objectMapper.readValue(new String(message.getPayload()), MqttSensorAlert.class);
            mqttSensorAlert.setSensorMqttName(getSensorNameFromTopic(topic));

            log.info("Save alert to DB: {}", mqttSensorAlert);
            if(sensorReadingService.addMqttAlert(mqttSensorAlert)) {
                log.info("Mqtt alert saved successfully to DB");
            }
            else {
                log.info("Mqtt alert saved unsuccessfully to DB");
            }
        } catch (JsonProcessingException | RuntimeException e) {
            log.error("Exception while handling new message: {}",e.getMessage());
            throw new RuntimeException(e);
        }
    }

    public void subscribeToTopic(String topic, Integer qos) throws MqttException {
        if (!subscribedTopics.contains(topic)) {
            mqttClient.subscribe(topic,qos);
            subscribedTopics.add(topic);
            log.info("Subscribed to topic: {}", topic);
        }
    }

    public void publishMessage(String topic, String payload, Integer qos, boolean retain) throws MqttException {
        log.info("Publishing mqtt message: {} to topic: {}", payload, topic);
        MqttMessage message = new MqttMessage(payload.getBytes());
        message.setQos(qos);
        message.setRetained(retain);
        mqttClient.publish(topic, message);
        log.info("Published message: {} to topic: {}", payload, topic);
    }

    public MqttCredentials configureMqttForNewSensor(Sensor sensor){
        String sensorMqttName = sensor.getSensorMqttName();
        String mqttClientId = sensorMqttName;
        String roleName = sensorMqttName+"_role";
        String password = passwordGenerator.getRandomPassword(10);

        String baseSensorTopic = "devices/"+sensorMqttName;
        String dataTopic = baseSensorTopic + "/data";
        String commandsTopic = baseSensorTopic + "/commands";
        String alertsTopic = baseSensorTopic + "/alerts";
        MqttAcl publishAcl = new MqttAcl("publishClientSend", dataTopic, Boolean.TRUE);
        MqttAcl alertAcl = new MqttAcl("publishClientSend", alertsTopic, Boolean.TRUE);
        MqttAcl subscribeAcl = new MqttAcl("subscribeLiteral", commandsTopic, Boolean.TRUE);
        List<MqttAcl> acls = List.of(new MqttAcl[]{publishAcl, alertAcl, subscribeAcl});
        MqttRole mqttRole = new MqttRole(roleName);

        CreateRoleCommand createRoleCommand = new CreateRoleCommand(roleName, acls);
        CreateClientCommand createClientCommand = new CreateClientCommand(mqttClientId, roleName, sensorMqttName, password, List.of(new MqttRole[]{mqttRole}));
        CommandsWrapper commandsWrapper = new CommandsWrapper(List.of(new MqttCommand[]{createRoleCommand, createClientCommand}));
        try {
            String payload = objectMapper.writeValueAsString(commandsWrapper);
            publishMessage(mqttDynamicSecurityTopic, payload, 0, false);

            log.info("Created mqtt client: {}", mqttClientId);
            return new MqttCredentials(mqttClientId, password);
        } catch (JsonProcessingException e) {
            throw new RuntimeException("Failed to serialize command: " + e.getMessage(), e);
        } catch (MqttException e) {
            throw new RuntimeException("Failed to publish MQTT message: " + e.getMessage(), e);
        }
    }

    public MqttCredentials resetMqttPasswordForSensor(Sensor sensor){
        String sensorMqttName = sensor.getSensorMqttName();
        String newPassword = passwordGenerator.getRandomPassword(10);

        MqttSetClientPassword modifyClientCommand = new MqttSetClientPassword(sensorMqttName, newPassword);
        CommandsWrapper commandsWrapper = new CommandsWrapper(List.of(new MqttCommand[]{modifyClientCommand}));
        try {
            String payload = objectMapper.writeValueAsString(commandsWrapper);
            publishMessage(mqttDynamicSecurityTopic, payload, 0, false);

            log.info("Modified mqtt client: {}", sensorMqttName);
            return new MqttCredentials(sensorMqttName, newPassword);
        } catch (JsonProcessingException e) {
            throw new RuntimeException("Failed to serialize command: " + e.getMessage(), e);
        } catch (MqttException e) {
            throw new RuntimeException("Failed to publish MQTT message: " + e.getMessage(), e);
        }
    }

    public Boolean setSensorReadingPeriod(String sensorMqttName, Integer readingPeriod){
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_READING_PERIOD, readingPeriod);
        return sendMqttSensorCommand(command, topic);
    }

    public Boolean setSensorState(String sensorMqttName, SensorState sensorState){
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_STATE, sensorState.getCode());
        return sendMqttSensorCommand(command, topic);
    }

    public Boolean setSensorHeight(String sensorMqttName, Integer height){
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_HEIGHT, height);
        return sendMqttSensorCommand(command, topic);
    }

    public boolean setSoilMoistureAlertThreshold(String sensorMqttName, Integer threshold) {
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_SOIL_MOISTURE_ALERT_THRESHOLD, threshold);
        return sendMqttSensorCommand(command, topic);
    }

    public boolean setSoilMoistureActionThreshold(String sensorMqttName, Integer threshold) {
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_SOIL_MOISTURE_ACTION_THRESHOLD, threshold);
        return sendMqttSensorCommand(command, topic);
    }

    public boolean setTemperatureAlertThreshold(String sensorMqttName, Integer threshold) {
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_TEMPERATURE_ALERT_THRESHOLD, threshold);
        return sendMqttSensorCommand(command, topic);
    }

    public boolean setTemperatureActionThreshold(String sensorMqttName, Integer threshold) {
        String topic = "devices/" + sensorMqttName + "/commands";
        MqttSensorCommand command = new MqttSensorCommand(MqttSensorCommandType.SET_TEMPERATURE_ACTION_THRESHOLD, threshold);
        return sendMqttSensorCommand(command, topic);
    }

//    TODO CHANGE QOS TO 2 FOR COMMANDS - adjust dynamic-security ACL
    private boolean sendMqttSensorCommand(MqttSensorCommand mqttSensorCommand, String topic) {
        try {
            String payload = objectMapper.writeValueAsString(mqttSensorCommand);
            publishMessage(topic, payload, 0, true);
            return true;
        } catch (JsonProcessingException | MqttException e) {
            throw new RuntimeException(e);
        }
    }

    public String getMqttBrokerUrl(){
        return this.brokerUrl.replace("tcp","mqtt");
    }

    public void removeSensorFromMqtt(Sensor sensor){
    }

    private String getSensorNameFromTopic(String topic){
        return topic.split("/")[1];
    }

}
