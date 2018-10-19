#!/bin/bash
g++ main.cpp -o test `pkg-config --cflags --libs opencv` -L. -lwiringPi -lGPIO -lpthread
