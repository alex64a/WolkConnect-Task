#!/bin/bash

# Copyright 2022 Wolkabout Technology s.r.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

docker container rm alex64a
#remove tmp if it exists and create it again
rm -rf tmp/
mkdir tmp
#build docker
docker build -t mydocker .

#run docker container
docker run -dit --name alex64a --cpus $(nproc) mydocker

#copy the debian from docker to the tmp/ folder
docker cp alex64a:/WolkConnect-Task/product/wolkconnect-task_1.0-1_amd64.deb tmp/




