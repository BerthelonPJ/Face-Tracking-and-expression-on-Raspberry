# Face Tracking and smileys w/ Raspberry and Arduino

Project made by Pierre-Jean Berthelon & Florian Bucheron.

Result and tutorial of this project can be found here : https://youtu.be/3uHDLBgpW8M

This project uses Raspberry, OpenCV libraries and servo motors to detect and follow a face on a video. Once a face is detected, we track it. The program also includes a smile and eyes detection, which allows to draw a smiley on the Raspberry's SenseHat module.  

This repository contains everything you need to run the project. 
Needed Hardware:
 - Raspberry model 3 (or other) 
 - SenseHat module
 - Raspicam
 - Arduino (Uno used in this project) with a 6V battery supply (the servos use too much power just for the usb)
 - Two Servo motors connected to the Arduino.
 
# Arduino/Raspian/OpenCV install:
 
# Arduino : 
sudo apt-get update 
sudo apt-get install arduino -y 
sudo usermod -a -G dialout votreNom -> s’obtient avec whoami
sudo usermod -a -G tty votreNom
 

Warning ! If synaptic is not installed : sudo apt-get install synaptic

# Qt Creator : 
sudo apt-get install qt-sdk
apt-get install tofrodos build-essential
cd ~/votre_rep/Qt-everywhere-opensource-4.6.3/
dos2unix configure
chmod 755 configure
./configure 
make
sudo make install
export PATH=$PATH:/usr/local/TrollTech/Qt-4.6.3/bin/ # a mettre dans votre .bashrc pour ne pas le refaire a chaque utilisation

# OpenCV : 
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config \libavcodec-dev libavformat-dev libswscale-dev
mkdir ~/src
cd ~/src
git clone https://github.com/opencv/opencv.git
cd opencv
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D INSTALL_PYTHON_EXAMPLES=ON \
      -D INSTALL_C_EXAMPLES=ON ..
make -j$(nproc)
sudo make install
pkg-config --cflags opencv  # get the include path (-I)
pkg-config --libs opencv    # get the libraries path (-L) and the libraries (-l)

Then you just need to:
  - Download the repository on your computer.
  - Run ProjetSY25Berthelon_Bucheron on QT creator (install opencv module before)
  - Run ControlMoteurArduino on Arduino IDE
  - Enjoy ! 
  
You can contact us here : 
  - pierrejean.berthelon@gmail.com
  - florian.bucheron@utt.fr

