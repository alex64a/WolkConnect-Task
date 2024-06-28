#!/bin/bash
tar -zcvf wolkconnect-task_0.0-1.orig.tar.gz --exclude=package --exclude=out --exclude=.git ../../WolkConnect-Task/
tar -zxvf wolkconnect-task_0.0-1.orig.tar.gz

cd WolkConnect-Task && debuild -us -uc -b -j${nproc}

