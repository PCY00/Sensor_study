#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>

#define I2C_DEVICE      "/dev/i2c-1"  // Jetson Nano의 I2C 포트
#define SLAVE_ADDRESS_1 0x57          // 첫 번째 아두이노의 I2C 주소
#define SLAVE_ADDRESS_2 0x60          // 두 번째 아두이노의 I2C 주소
#define WRITE_REGISTER  0x01          // 사용자 명령어를 보내기 위한 레지스터 주소
#define READ_REGISTER   0x02          // start 신호를 보내 슬레이브에게 데이터를 읽어오라고 요청하기 위한 레지스터 주소
#define START_SIGNAL    0xA0          // start 신호를 보내기 위한 명령어

// I2C 장치 열기
// 주어진 주소로 I2C 장치를 열고, 슬레이브 주소를 설정하는 함수
int openI2CDevice(int address) {
    int f = open(I2C_DEVICE, O_RDWR);  // I2C 장치 파일 열기
    if (f < 0) {
        std::cerr << "I2C 장치를 열 수 없습니다." << std::endl;
        return -1;
    }

    // 슬레이브 주소 설정
    if (ioctl(f, I2C_SLAVE, address) < 0) {
        std::cerr << "슬레이브 주소를 설정할 수 없습니다." << std::endl;
        close(f);
        return -1;
    }
    return f;
}

// 데이터 수신 함수
// I2C 슬레이브 장치로부터 데이터를 읽어오는 함수
bool readData(int file) {
    unsigned char buffer[1] = {START_SIGNAL};  // 슬레이브에 데이터 요청 신호

    // 슬레이브에게 데이터 요청 신호 전송
    if (write(file, buffer, 1) != 1) {
        std::cerr << "데이터 전송에 실패했습니다." << std::endl;
        return false;
    } else {
        std::cout << "데이터 전송 성공" << std::endl;
    }

    unsigned char data;
    // 슬레이브로부터 데이터 읽기
    if (read(file, &data, 1) != 1) {
        std::cerr << "데이터 수신에 실패했습니다." << std::endl;
        return false;
    } else {
        std::cout << "받은 데이터: " << data << std::endl;
    }
    return true;
}

// 사용자 명령어 전송 함수
// 사용자가 입력한 명령어를 I2C를 통해 슬레이브 장치에 전송하는 함수
void sendCommand(int file, const std::string &command) {
    size_t bufferSize = 1 + command.size();  // 버퍼 크기: 레지스터 1바이트 + 명령어 문자열 크기
    unsigned char *buffer = new unsigned char[bufferSize];
    
    buffer[0] = WRITE_REGISTER;  // 첫 번째 바이트에 레지스터 주소 저장
    std::memcpy(buffer + 1, command.c_str(), command.size());  // 명령어 문자열을 버퍼에 복사

    // 슬레이브 장치로 명령어 전송
    if (write(file, buffer, bufferSize) != bufferSize) {
        std::cerr << "데이터 전송에 실패했습니다." << std::endl;
    } else {
        std::cout << "명령어 '" << command << "'을(를) 보냈습니다." << std::endl;
    }

    delete[] buffer;  // 동적 메모리 해제
}

// 프로그램 종료 시 클린업
// 프로그램이 종료될 때 실행되어 자원을 정리하는 함수
void cleanup(int signum) {
    std::cout << "\n프로그램을 종료합니다." << std::endl;
    exit(0);
}

// 명령어 파싱 함수
// 사용자가 입력한 명령어를 파싱하여 아두이노 번호와 명령어 내용을 분리하는 함수
bool parseCommand(const std::string &command, int &arduinoNum, std::string &action) {
    if (command.size() < 3 || command[0] != 'M' || command[2] != '_') return false;

    arduinoNum = command[1] - '0';  // 두 번째 문자에서 아두이노 번호 추출
    action = command.substr(3);     // 세 번째 문자부터 명령어 추출

    return true;
}

int main() {
    signal(SIGINT, cleanup);  // Ctrl+C 입력 시 프로그램 종료를 위해 시그널 핸들러 등록

    std::string command;      // 사용자로부터 입력받을 명령어
    int address, arduinoNum;  // 아두이노 주소와 번호
    std::string action;       // 명령어 내용

    std::cout << "M?_ON, M?_OFF, M?_D??으로 명령을 줄 수 있습니다.\n?는 아두이노 번호이고, ??는 깜빡이 딜레이(초)입니다." << std::endl;

    while (true) {
        std::cout << "명령을 입력하세요: ";
        std::cin >> command;  // 사용자로부터 명령어 입력받기

        // 명령어 파싱
        if (!parseCommand(command, arduinoNum, action)) {
            std::cerr << "잘못된 명령 형식입니다." << std::endl;
            continue;
        }

        // 아두이노 번호에 따라 주소 설정
        if (arduinoNum == 1) {
            address = SLAVE_ADDRESS_1;
        } else if (arduinoNum == 2) {
            address = SLAVE_ADDRESS_2;
        } else {
            std::cerr << "잘못된 아두이노 번호입니다." << std::endl;
            continue;
        }

        // I2C 장치 열기
        int file = openI2CDevice(address);
        if (file < 0) continue;

        // 명령어 내용에 따른 동작 수행
        if (action == "P1_1000") {
            sendCommand(file, command);  // 특정 명령어일 경우 전송
        } else if (action == "READ") {
            readData(file);  // READ 명령어일 경우 데이터 읽기
        } else {
            std::cerr << "알 수 없는 명령입니다." << std::endl;
        }

        close(file);  // I2C 장치 닫기
    }

    cleanup(0);  // 프로그램 종료 시 자원 정리
    return 0;
}
