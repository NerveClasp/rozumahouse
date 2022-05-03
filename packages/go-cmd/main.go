package main

import (
	"encoding/json"
	"fmt"
	mqtt "github.com/eclipse/paho.mqtt.golang"
	"log"
	"net/url"
	"os"
	"strconv"
	"time"
)

type brightnessChange struct {
	Action     string `json:"action"`
	Brightness int    `json:"brightness"`
	Led        int    `json:"led"` // 1
}

func connect(clientId string, uri *url.URL) mqtt.Client {
	opts := createClientOptions(clientId, uri)
	client := mqtt.NewClient(opts)
	token := client.Connect()
	for !token.WaitTimeout(3 * time.Second) {
	}
	if err := token.Error(); err != nil {
		log.Fatal(err)
	}
	return client
}

func createClientOptions(clientId string, uri *url.URL) *mqtt.ClientOptions {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(fmt.Sprintf("tcp://%s", uri.Host))
	opts.SetUsername(uri.User.Username())
	password, _ := uri.User.Password()
	opts.SetPassword(password)
	opts.SetClientID(clientId)
	return opts
}

func listen(uri *url.URL, topic string) {
	client := connect("sub", uri)
	client.Subscribe(topic, 0, func(client mqtt.Client, msg mqtt.Message) {
		fmt.Printf("* [%s] %s\n", msg.Topic(), string(msg.Payload()))
	})
}

func main() {
	uri, err := url.Parse(os.Getenv("MQTT_SERVER"))
	if err != nil {
		log.Fatal(err)
	}
	topic := uri.Path[1:len(uri.Path)]
	if topic == "" {
		topic = "test"
	}

	go listen(uri, topic)

	client := connect("pub", uri)

	message := "{}"

	command := os.Args[1]
	// arguments := os.Args[2:]

	if command == "brightness" || command == "b" {
		brightness := os.Args[2]
		bInt, err := strconv.Atoi(brightness)
		if err != nil {
			log.Fatal(err)
		}
		messageD := &brightnessChange{Action: "command", Brightness: bInt, Led: 1}
		messageJ, _ := json.Marshal(messageD)
		message = string(messageJ)
		client.Publish(topic, 0, false, message)
	}

	timer := time.NewTicker(1 * time.Second)
	for t := range timer.C {
		fmt.Println(t)
		client.Publish(topic, 0, false, message)
		return
	}
}
