FROM ubuntu:20.04

WORKDIR /

#set timezone for Europe/Belgrade
ENV TZ=Europe/Belgrade
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

#Install libsensors4-dev for reading temperatures of CPU cores
RUN apt-get update -y && apt-get install libsensors-dev -y
RUN apt update -y && apt install cmake g++ libssl-dev libpthread-stubs0-dev git lm-sensors -y

#Copy all files from current directory of host machine to the docker image
COPY . .
RUN mkdir out
WORKDIR /out
#Create out directory and build
RUN cmake .. && make -j6

#Port 1883 for MQTT protocol
EXPOSE 1883

CMD ["echo","Dockerfile by alex64a"]