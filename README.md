<a name="br1"></a> 

**CS3237 Introduction to Internet of Things Project**

**Smart Lamp Working Assistant**

**Final Report**

Yu Haihong A0211200M

Jonathan Mui A0217498N

Joel Wong A0234053R

Alwin Ang A0217735B

Julian Wong Wei Yin A0223486B

1



<a name="br2"></a> 

**Section 1 Problem Statement.....................................................................................................4**

**Section 2 Hardware Design........................................................................................................ 4**

**Section 3 Software Implementation...........................................................................................6**

Section 3.1 Actuation Programming........................................................................................ 6

Section 3.1.1 Actuation Design and Precedence...............................................................6

3\.1.2. Actuation Periodic Pattern....................................................................................... 8

Section 3.2 Communication Protocols.....................................................................................9

Section 3.2.1 ESP32CAM..................................................................................................9

Section 3.2.2 ESP32: Smart Lamp.................................................................................. 12

Section 3.3 Concurrency....................................................................................................... 13

Section 3.4 Power Saving......................................................................................................14

Section 3.4.1 ESP32CAM................................................................................................14

Section 3.4.1 ESP32: Smart Lamp.................................................................................. 14

**Section 4 Machine Learning..................................................................................................... 14**

Section 4.1 Data collection.................................................................................................... 15

Section 4.2 Data cleaning and Model Training...................................................................... 16

Section 4.3 Selected model and Transfer Learning...............................................................16

Section 4.4 Model’s test accuracy......................................................................................... 16

Section 4.5 Model Deployment..............................................................................................17

**Section 5 Backend AWS Cloud Network................................................................................. 18**

Section 5.1 Flow of data........................................................................................................ 18

Section 5.2 Security...............................................................................................................18

Section 5.3 Cost Analysis......................................................................................................19

**Section 6 Challenges Faced..................................................................................................... 20**

Section 6.1 Integration.....................................................................................................20

**Section 7 Possible Future Directions...................................................................................... 21**

**Bibliography...............................................................................................................................22**

2



<a name="br3"></a> 

**Section 1 Problem Statement**

For many, spending long hours sitting at a desk is inevitable. According to the National

University Hospital, approximately 37% of Singaporeans spend at least 8 hours a day sitting

down. [1] Bad posture while sitting leads to negative health effects like neck, back and shoulder

pain, stress incontinence, and heartburn. [2] Hence, it is important to maintain good posture

while sitting to avoid exacerbating the problems brought about by long periods of sitting. We

have developed an IoT device that reminds the user to have better posture.

Most products developed to solve these problems are physical braces to pull the user’s

shoulders into proper alignment. [3] There currently exists an IoT solution – Upright Go, a

device that is attached to the user’s back and vibrates when bad posture is detected. Data on

the user’s posture is sent by the device to the user’s phone where statistics can be viewed and

settings for the device can be changed. While the device provides real-time haptic feedback, the

wearable nature of the device has caused some issues for users, mainly the adhesive attaching

the device to the user’s back failing. [4]

In order to address this problem, we came up with an IoT device that is both a desktop lamp

and a working assistant. Equipped with an ESP32CAM, it can capture the user’s posture and

activity. According to the combinations of posture and activities, the lamp would auto-adjust its

light mode, together with other actuations , such as the OLED display and Telegram Bot

notification.

**Section 2 Hardware Design**

To advocate for sustainability, we constructed the external enclosure using recycled cardboard.

Additionally, we adopted a modular strategy, allowing the camera, light, and control system to be

individually removed, facilitating more straightforward maintenance.

3



<a name="br4"></a> 

The sensors and actuators used are as follows

Component name purpose

Sensors

ESP32CAM

Photoresistor

LED ring

Capture user’s posture and activity, then send to the could

for machine learning classification

Measure the environmental brightness, send to ESP32 for

LED brightness control

Actuators

Provide lighting for user. The lighting mode, i.e. color and

brightness, varies according to different scenarios.

OLED screen

Display an lively robot face with expressions that varies

according to different scenarios. It brings the

interestingness and interactiveness to users.

Telegram Bot

When there’s a monitor of bad posture, there will be a push

notification to the Telegram Bot to remind user.

We used a DOIT ESP32 Devkit V1 to control the LED ring and OLED screen. It also monitors

the surrounding brightness through photoresistor. The ESP32CAM is equipped with a

microcontroller itself, and the Telegram Bot is controlled from the server side. The detailed

connection of all components is as follows.

Remarks

1\. The OLED screen used I2C communication.

4



<a name="br5"></a> 

2\. The LED ring model WS2812 has 3 pins, 5V power, GND, and a digital input. Each of

the LED bead on the ring is individually addressable, whose color and brightness can be

controlled by the input pin.

3\. A voltage divider is designed for the photoresstor with a 330 Ohm resistor. The voltage

that can be read by pin 33 is calculated as follows:

330 푂ℎ푚 × 3.3 푉

푉

\=

33

푅

+330 푂ℎ푚

푝ℎ표푡표푟푒푠푖푠푡표푟

A special consideration is the pin selection for analog reading. ESP32 has two ADCs.

ADC2 is actively used by the WiFi. Therefore, we can only use ADC on any of the ADC1

channel, which uses pins GPIO36, GPIO37, GPIO38, GPIO39, GPIO32, GPIO33,

GPIO34 and GPIO35 [1].

**Section 3 Software Implementation**

**Section 3.1 Actuation Programming**

**Section 3.1.1 Actuation Design and Precedence**

As shown in the flow chart below, according to different activity-posture combinations, different

actuations would be set for both the LED ring and the OLED screen.

The classified activity and posture are published on 2 separate MQTT topics. There are 2 flags

to indicate the received message, one called activity, and the other called posture. Upon

receiving the messages, flags are set as below.

5



<a name="br6"></a> 

Topic

Message

Flag

laptop/posture “good\_posture”

“bad\_posture”

posture = 1

posture = 2

activity = 1

activity = 2

activity = 3

activity = 4

laptop/activity "computer\_use"

“reading”

“asleep”

“not\_present”

A series of if-else statements are then build upon the flag values, to ensure a strict precedence

of the actions taken. The program will first check if the activity is equal to 3 or 4, whose action

takes the 1st priority. Then it would check if posture equals to 2, whose action takes 2nd priority.

FInally, the combination of posture=1 and activity=1 or 2 takes the lowest priority.

Using the FastLED library, each LED bead’s brightness and color can be controlled. For

example:

for (int i = 0; i < NUM\_LEDS; i++) { leds[i] = CRGB::White; }

FastLED.setBrightness(brightness);

The robot face expression animation is implemented using Adadruit SSD1306 Wemos Mini

OLED library. There are 3 expressions designed: blinking, sleeping, and angry. Each of the key

frame is a 64\*48 .png picture. They are then converted to bitmap using bitmap\_conversion.py

file. All the converted bitmaps are stored in arrays named after their key frames in the

OLEDImage.h file. Example of drawing a frame on OLED is as follows:

void draw(const unsigned char \*experession) {

display.drawBitmap(0, 0, experession, 64, 48, 1);

display.display();

display.clearDisplay();

}

Below is a table of the key frames designed for each expression.

Experssion Key Frame

start

blink1

blink2

blink

start

angry1

angry2

angry3

angry4

6



<a name="br7"></a> 

angry

start

sleep1

sleep2

sleep3

sleep4

sleep5

sleeping

**3.1.2. Actuation Periodic Pattern**

Some of the actuations, such as breathing light, flashing light, and OLED animation, include a

periodic pattern that needs to be constantly repeated. However, we avoided using multiple loops

with delay, since it will hold the program in one loop and might miss out on the next trigger, but

we want a system that gives real-time feedback.

Therefore, considering that 1000 ms is the least common multiple of all the actuation’s period,

we implemented a counter that counts from 0 to 999, and between each count, there’s a delay

of 1 ms. The counter is then used as a condition to trigger the periodic actuation. For example,

doing something when counter%100 equals 0, would allow something to repeat every 100ms.

The detailed implementation for each actuation will be explained below.

a. if activity = 4 (not present)

Both **LED** and **OLED** are turned off.

b. else, if activity = 3 (sleeping)

**LED:** yellow breathing light. The brightness would increase by 1 every 100 ms, until it

reaches 10; then brightness decrease by 1 every 100 ms, until it reaches 0.

The increase and decrease happens when counter%100 = 0. There’s also a “flag\_up”

indicating whether it’s increasing or decreasing.

**OLED:** repeated animation of sleeping. Each frame are separated by a 200ms interval.

There is a transition stage followed by a repeated stage, the sequence are as follows.

7



<a name="br8"></a> 

c. else, if posture = 2 (bad posture detected)

**LED:** read flashing light. The brightness increases by 1 every 1 ms, until it reaches 254;

then brightness decreases by 1 every 1 ms, until it reaches 0.

There’s also a “flag\_up” indicating whether it’s increasing or decreasing.

**OLED:** repeated animation of angry, transition stage followed by repeated stage.

d. else (good posture and in reading/computer)

**LED:** the LED would be turned on, with brightness varies according to surrounding

environment. To achieve, we took the anlogRead value from the photoresistor pin, where

the values ranges from 0 to 4095, and is positively proportional to the environment

brightness. Therefore, the following equation is used to calculate the brightness for LED

to be set:

BRIGHTNESS = max(96 + 1 \* (600 - analogRead(PHOTO\_PIN)), 0.0);

The default LED brightness is 96, and threshold for photoresistor is 600. When reading

for photoresistor is smaller than 600, the environment is considered too dark, and the

BRIGHTNSS would increase to provide sufficient lighting, and vice versa.

When activity=1 (computer\_use), LED is set to white color. When activity=2 (reading),

LED is set to warm yellow color.

**OLED:** repeated animation of blinking.

**Section 3.2 Communication Protocols**

**Section 3.2.1 ESP32CAM**

The microcontroller of choice for taking photos is the ESP32CAM. The ESP32CAM is a low-cost

microcontroller with a camera module. It also retains many core features of the more commonly

used DOIT ESP32 DevKit V1, such as WiFi, GPIO, common communication protocols

8



<a name="br9"></a> 

(UART/SPI/I2C/PWM/ADC/DAC), and embedded FreeRTOS. Given a more conservative

budget for this project, the ESP32CAM fits the system requirements.

Fig XXX: ESP32CAM program flow

The overall flow for the ESP32CAM is shown in Figure XXX. The ESP32CAM only needs to

take a photo and send it over to the backend via AWS IoT Core; it does not need to do anything

else, or receive any messages from AWS IoT Core. To preserve the limited 1000mAh power

bank, the ESP32CAM is put into a deep sleep after the photo is published to AWS IoT Core.

This program flow is reliable as any connection that is unable to be established will halt the flow

of the program until successful. Hence, there will not be a case where the photo is taken and

attempted to be sent over AWS IoT Core while a connection to AWS IoT Core is not

established.

Placing the ESP32CAM in deep sleep to preserve the power source presents some challenges.

To keep the connection with AWS IoT Core alive while the photo is being taken, encoded and

sent, a *mqttClient.loop()* function (assume the client object variable is named *mqttClient*) must

9



<a name="br10"></a> 

be called regularly. In most scenarios, this can be simply done in the *void loop()* function of the

Arduino sketch, which is sufficient to keep the connection alive. However, with the deep sleep

functionality implemented, *void loop()* can no longer be used. Hence, the embedded FreeRTOS

capability on the ESP32CAM allows for a convenient workaround, given that keeping the MQTT

client connection alive is the only looping function needed.

void mqttLoopTask(void\* pvParameters) {

Serial.print("task running on core ");

Serial.println(xPortGetCoreID());

for (;;) {

if (!mqttClient.connected()) {

Serial.println("MQTT disconnected. Attempting to reconnect...");

connectToAWS(); *// Reconnect to AWS*

} else {

mqttClient.loop();

}

vTaskDelay(pdMS\_TO\_TICKS(1000)); *// Delay to free up CPU cycles*

}

}

This function runs on one of the two available cores of the ESP32CAM. It keeps the client

connection alive. If the connection happens to be lost, it will repeat the process of connecting to

AWS IoT Core. A point to take note is that since running a RTOS task is independent of the

standard loop function, even if the ESP32CAM is placed in deep sleep, this task will still be kept

running on the core. Hence, the task must be manually killed by setting the Task Handler of the

RTOS task to NULL. The ESP32CAM can then be placed into deep sleep.

10



<a name="br11"></a> 

**Section 3.2.2 ESP32: Smart Lamp**

For the lamp’s ESP32, the two tasks related to MQTT communications are handleConnect and

handleLoop. In order for the connection between the ESP32 and the AWS MQTT server to be

maintained, the mqttClient.loop() function needs to be called periodically. Hence, it was placed

in a designated task handleLoop meant to run forever, calling the mqttClient.loop() function if the

ESP32 is connected to the AWS MQTT server.

The handleConnect task handles the connections and disconnections to WiFi and the MQTT

server. If the ESP32 is connected to WiFi and the AWS server, the task is put into the Blocked

state for 10000 milliseconds to allow for the ESP32 to receive any new incoming messages

from the AWS server. This means the task does not progress for 10 seconds, while allowing the

core to run other tasks during that time. After the task is unblocked, the ESP32 disconnects

from the MQTT server and from Wifi.

An interrupt sets the flag initiateConnect every 20 seconds. If the flag is set and the WiFi is not

already currently connected, the initiateConnect flag is reset, and the connectToWiFi() and

connectToAWS() functions are called.

In the connectToWiFi() function, the ESP32 connects to the WiFi hotspot of a mobile phone

which acts as a gateway device.

11



<a name="br12"></a> 

In the connectToAWS() function, the WiFi Client of the ESP32 is configured to use the AWS IoT

device credentials. The connection to the AWS MQTT server is initiated. The message handler

is created. The relevant topics are subscribed to.

After the ESP32 connects to the AWS MQTT server, the message handler function is initialised

and the topics related to activity and posture are subscribed to. The classifier publishes its

messages in the form of a JSON key-value pair encoded as a string. When a message with a

subscribed topic is published to the server, the message handler of the ESP32 deserialises the

payload of the message, and obtains the value as the message. For messages with topic

“laptop/activity”, the activity flag is set, and for messages with topic “laptop/posture”, the posture

flag is set. These set flags then affect the actuation state of the device.

Our message protocol between our ESP32 and the AWS lambda is not synchronised. Hence it

is not guaranteed that the classifier publishes a message during the window in which the ESP32

is connected to the MQTT server. To ensure the ESP32 is still able to receive the latest

classification, the AWS lambda sends retained messages to the MQTT server. When the AWS

lambda publishes a retained message, the server stores it as the most recent message for that

topic. The ESP32 immediately receives the retained messages when subscribing to the retained

message’s topic upon connection to the server.

**Section 3.3 Concurrency**

Our implementation makes use of FreeRTOS, an open source real time operating system kernel

that is integrated into ESP-IDF (Espressif IoT Development Framework).

There are some delays incurred in the functions related to communications, such as when the

ESP32 connects to the gateway device via WiFi and when connecting to the AWS MQTT

server, which can last from a few hundred milliseconds to several seconds.

12



<a name="br13"></a> 

These delays cause problems in functioning in other parts of our code. The function

mqttClient.loop() has to be called periodically while the ESP32 is connected to the MQTT server

to keep the connection between the device and the server alive. We have estimated the

required frequency to be at least once every 1000 ms from our experimental tuning.

Furthermore, the state of our peripherals may need to change at a very quick frequency. For

example, to get the desired functioning of the lamp in the “Warning” state when the user is

detected to have bad posture, the brightness of the LEDS increments or decrements every CPU

clock cycle.

To avoid any delay in the functions related to communications from affecting the timing of the

peripherals, the tasks related to Wifi and MQTT are run on one core and tasks related to the

peripherals are run on the other core.

**Section 3.4 Power Saving**

**Section 3.4.1 ESP32CAM**

The ESP32CAM is put into deep sleep for 20 seconds after the photo is taken and sent to the

cloud via AWS IoT Core. According to the ESP32CAM’s datasheet, at an input voltage of 5V the

device consumes 180mA during normal operation, and 6mA when in deep-sleep. On average,

for every 20 seconds the device is in deep sleep it is in active mode for 7 seconds to execute all

the functionalities required of it.

퐵푎푡푡푒푟푦 퐶푎푝푎푐푖푡푦

1000푚퐴ℎ

퐸푠푡푖푚푎푡푒푑 퐵푎푡푡푒푟푦 퐿푖푓푒 =

\=

= 19. 52ℎ

퐴푣푒푟푎푔푒 푐푢푟푟푒푛푡 푑푟푎푤푛

7

20

180푚퐴 \*

\+ 6푚퐴 \*

7 + 20

20 + 7

**Section 3.4.1 ESP32: Smart Lamp**

We opted to disconnect the ESP32 from WiFi when it was not needed instead of putting it into

sleep mode as the CPU would be paused or not running in the various sleep modes the ESP32

is capable of, and we required it to continue running to handle the peripheral actuations. Our

lamp consumes a quarter of a 1000mAh battery in an hour, so assuming an ideal battery our

lamp will last 4 hours on a 1000mAh battery.

**Section 4 Machine Learning**

Solving the issues raised in the problem statement, our product needs to classify 2 things from

the images taken from the ESP32 camera: the user’s activity and the user’s posture. We

decided to use a Convolutional Neural Network (CNN) as this deep learning model excels in

image classification tasks.

13



<a name="br14"></a> 

**Section 4.1 Data collection**

In order to train our model to classify our 4 activities (reading, computer use, sleeping, not

present) and 2 postures (good posture, bad posture), we took photos of us performing these

actions and searched for stock images online of people performing the same actions as well.

We eventually collected about 200 images per class.

not\_present

reading

sleeping

computer\_use

good\_posture

bad\_posture

14



<a name="br15"></a> 

**Section 4.2 Data cleaning and Model Training**

After collecting the data, we wanted to take the multilabel classification approach by one-hot

encode each image with its corresponding labels (eg: 010001 if the user is reading with bad

posture). However, after performing a few training runs with the one-hot encoded data and

getting suboptimal results, we concluded that we probably do not have enough time (given the

timeline of the project) to collect the amount of data required for the model to accurately perform

multilabel classification. We thus changed our approach to a single label classification and

decided to use 2 separate models to classify the user activity and user posture separately. The

model architecture used for both classification tasks will however be exactly the same (as

explained in Section 4.3).

We also standardized the image size to 128x128 and did a random horizontal flip of the images

before they are fed into the models. These techniques are meant to ensure that the image

dimensions match the input nodes of the model and that the model will not be overfitted to only

recognise images from 1 side of the table.

**Section 4.3 Selected model and Transfer Learning**

Since posture and activity detection in images is similar to object detection tasks, we decided to

perform Transfer Learning for our classification tasks using a deep CNN model, ResNet-18.

ResNet-18 is a CNN with 18 deep layers designed to allow large numbers of convolutional

layers to work efficiently. It has also been pretrained on the large ImageNet dataset and can

classify images into 1000 object categories such as animals, keyboards, mice and pencils. By

tweaking the final output layer and training it on our image dataset, we would be able to get

accurate classification results without the need for long training times and large datasets.

ResNet 18 Model Architecture

**Section 4.4 Model’s test accuracy**

The following images show the confusion matrix of the trained models’ accuracy in classifying

the user’s activity and posture respectively.

15



<a name="br16"></a> 

**Section 4.5 Model Deployment**

Our trained model is subsequently deployed on the cloud through a flask server that is hosted

on an AWS lightsail instance running an Ubuntu server. The AWS lambda functions running the

backend of our system can then classify the user activity or user posture from the images on our

ESP32 by sending an http POST request to the respective routes (“/classify\_activity” or

“/classify\_posture).

16



<a name="br17"></a> 

**Section 5 Backend AWS Cloud Network**

To address the limitations of the ESP32 and efficiently handle image processing, machine

learning model execution, and communication with ESP32s, we've opted for a cloud-based

solution, specifically leveraging Amazon Web Services (AWS) as a Platform as a Service

(PaaS) provider. This strategic choice aligns with our project's requirements while benefiting

from the AWS free tier for cost-effectiveness. The three primary tasks involve storing images

and classifications, running the machine learning model, and facilitating communication with

ESP32s via MQTT.

**Section 5.1 Flow of data**

The start of the data flow is from the ESP32 Camera sending the encoded base64 image to the

AWS IOT Core MQTT Topic esp32/pub. An AWS Lambda Function (ImageDecoderLambda) will

listen to this topic and receive the bytes to convert the bytes to be uploaded to a AWS S3

Bucket. This uploading of the image will trigger the AWS Lambda Function

(ClassficationLambda) to take that image and make an API call to an AWS Lightsail image

containing the 2 Machine Learning Models to classify the posture and activity. It takes the 2

results and sends them to 3 endpoints, Google Firestore for long term storage, Telegram Bot (if

bad posture detected) and IOT Core MQTT Topics laptop/activity and laptop/posture which the

ESP32 Lamp is listening to and receives the classification.

**Section 5.2 Security**

We opted for AWS Web Services primarily because of its robust and customizable security

infrastructure. AWS provides users with a high degree of flexibility in tailoring security measures,

enabling us to fine-tune the level of security that aligns with our specific requirements. This

flexibility is invaluable during both the testing and deployment phases of our backend

development.

17



<a name="br18"></a> 

**Section 5.2.1 IAM Roles**

We have diligently applied the principle of least privilege to our Lambda functions. The

ImageDecoderLambda role is tailored to exclusively subscribe to the AWS IoT Core's topic and

execute PUT actions into the specified S3 bucket. Conversely, the ClassificationLambda role is

crafted with limited permissions, allowing only GET access to retrieve images from the S3

bucket. This targeted approach ensures each Lambda function has precisely the permissions it

needs, enhancing the security of our serverless architecture.

**Section 5.2.2 Certificates**

In order to connect to the AWS IOT Core topic, an adversary has to have gotten the IOT Core

Thing certificate which is uploaded into the ESP32. Even if the adversary has gotten the

certificate, ImageDecoderLambda which is the first edge of the backend, checks if the received

message is a valid jpg file. Thus an attacker cannot upload arbitrary payloads to the S3 bucket

and overload the S3 bucket.

**Section 5.2.3 Cloudwatch**

AWS CloudWatch is seamlessly integrated into each tier of our backend, enabling real-time

monitoring of actions performed. With logs subscribed to every edge, we gain immediate

insights into the timestamp and nature of data processing. This proactive approach allows us to

swiftly identify and respond to any unauthorised access or irregularities within our edge things,

ensuring a heightened level of security.

**Section 5.3 Cost Analysis**

Costs of AWS Services being used

AWS IOT Core

AWS S3 Bucket

AWS Lambda

AWS ECR

250,000 actions Free Free Unlimited Data

400,000 GB-seconds Free 500mb of

/ month storage / month

/ month

Transfer to other

services in same

region

2,250,000 minutes of $0.025 USD per GB

connection Free /

month

for storage

**Section 5.3.1 AWS IOT Core**

In each cycle, there would be 2 calls to the MQTT broker, one to receive the bytes from the

ESP32 Camera, and another to send the classification for the Posture and Activity. Considering

we are having a trigger every 20 seconds to take a picture from the IOT Core and the ESP32

Lamp waits for a message for at most 10 seconds so we can see that the connection will be 11

seconds of connection every 30 seconds if out of alignment. In the longest month (31 days) we

18



<a name="br19"></a> 

have 44640 minutes and if we connect only 11/30 of the time, we only have 16368 minutes of

connection which is free.

**Section 5.3.2 AWS S3 Bucket**

In our cloud backend, we can see that the S3 Bucket gets an image upload of 10 - 20 kb (15kb

on average) every 20 seconds and sends that image to a AWS Lambda within the same region.

This translates to 44640 minutes \* 3 cycles per minute \* 15kb = 2008800 kb / 2.0088 GB of data

/ month or just $0.06 USD / month. For the Lambda retrieval since it is within the same region, it

will be free.

**Section 5.3.3 AWS Lambda**

ImageDecoderLambda requires 230ms of 77 MB (0.0177GB/s) / call and ClassficationLambda

requires 1799ms of 139 MB (0.25 GB/s) / call. If we do 3 calls each per min, we have a total of 3

\* (0.0177 + 0.25) GB/s \* 44640 = 35,850.384 GB/s which is free.

**Section 5.3.4 AWS ECR**

This is a one time cost as the AWS ECR is used to store the Docker image to be deployed for

the ClassficationLambda and the image is on average 280mb and we only require 1 image.

Thus it is also free.

The total cost to run 3 cycles / minute for a 31 days month is $0.06 USD or $0.08 SGD.

**Section 6 Challenges Faced**

**Section 6.1 Integration**

One of the main challenges was during the integration phase of the project. Each individual

component was not too difficult to set up locally, but having data flow through the whole pipeline

during integration proved to be difficult. There were quite a few permissions and settings hidden

in the AWS backend that needed to be set correctly in order for data to pass through from the

microcontrollers. While dealing with the RTOS tasks, several race conditions led to frequent

disconnections from AWS IoT Core and the ESP32 on the lamp was unable to receive

classifications from the ML model.

To overcome this, we followed common concurrency practices such as ensuring that no tasks

sharing the same core were blocking in nature, and used non-blocking delays which would yield

control to the other task when idle. Variables shared between functions in and outside of the

software interrupt were also declared *volatile*. We tested the entire AWS-related section of the

pipeline with dummy data (for example, manually placing a photo into the S3 bucket), and

checking if the classification is produced by the last AWS Lambda function in the pipeline. After

these implementations, a complete pipeline was successfully set up.

19



<a name="br20"></a> 

**Section 7 Possible Future Directions**

We have collected the classifications and our timestamps from our camera data in Google

Firestore. This data can be further used to improve the efficiency of our device. For example, a

regression model can be used with our data to predict time periods where no one is present in

the room, so that data can be collected less often. The camera could be configured to only

come out of deep sleep every minute instead of every 20 seconds during this predicted time

period. This could help to further reduce the power consumption and increase the battery life of

our device.

20



<a name="br21"></a> 

**Bibliography**

[1] How to combat sedentary behaviour or the “sitting disease”?: NUHS. Default. (2022, March 20).

<https://nuhsplus.edu.sg/article/how-to-combat-sedentary-behaviour-or-the-sitting-disease>

[2] 3 surprising risks of poor posture. Harvard Health. (2023, July 20).

<https://www.health.harvard.edu/staying-healthy/3-surprising-risks-of-poor-posture>

[3] Roberts, C. (2023, July 14). *Are posture-correcting devices helpful? straight talk from experts.* The

Washington Post.

<https://www.washingtonpost.com/wellness/2023/07/17/posture-devices-back-pain/>

[4] *The upright go 2 is out... with the problems of the original*. Leaf&Core. (2019, June 16).

<https://leafandcore.com/2019/06/16/the-upright-go-2-is-out-with-the-problems-of-the-original/>

21

