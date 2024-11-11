sudo apt update
sudo apt install -y build-essential g++
sudo apt install -y cmake pkg-config
sudo apt install -y libxml2-dev libxslt-dev
sudo apt-get install git
sudo aot-get install curl zip unzip tar 
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./vcpkg install libxml2


git clone https://github.com/zweistein22/rarcrack.git
cd rarcrack
cmake .
cd bin
