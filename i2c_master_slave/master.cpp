#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <cstring>
#include <chrono>
#include <thread>

#define I2C_DEVICE      "/dev/i2c-1"  // Jetson Nano의 I2C 포트
#define SLAVE_ADDRESS_1 0x57          // 첫 번째 아두이노의 I2C 주소
#define SLAVE_ADDRESS_2 0x60          // 두 번째 아두이노의 I2C 주소
#define WRITE_REGISTER  0x01          // LED 제어를 위한 레지스터 주소
#define READ_REGISTER   0x02          // LED 상태를 읽기 위한 레지스터 주소
#define LED_ON          0xAC          // LED 켜기 명령
#define LED_OFF         0x1F          // LED 끄기 명령

int file;  // I2C 파일 디스크립터

// I2C 장치 열기
int openI2CDevice(int address) {
    int f = open(I2C_DEVICE, O_RDWR);
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

// LED 제어 함수
void controlLED(int file, bool turnOn) {
    unsigned char buffer[2];
    buffer[0] = WRITE_REGISTER;
    buffer[1] = turnOn ? LED_ON : LED_OFF;

    if (write(file, buffer, 2) != 2) {
        std::cerr << "데이터 전송에 실패했습니다." << std::endl;
    } else {
        std::cout << "LED " << (turnOn ? "켜기" : "끄기") << " 명령을 보냈습니다." << std::endl;
    }
}

// 깜빡임 제어 함수
void blinkLED(int file, int delaySeconds) {
    for (int i = 0; i < 2; ++i) {  // 2번 깜빡임
        controlLED(file, true);
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
        controlLED(file, false);
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
    }
}

// 프로그램 종료 시 클린업
void cleanup(int signum) {
    if (file >= 0) {
        close(file);
    }
    std::cout << "\n프로그램을 종료합니다." << std::endl;
    exit(0);
}

// 명령어 파싱 함수
bool parseCommand(const std::string &command, int &arduinoNum, std::string &action, int &delaySeconds) {
    if (command.size() < 4 || command[0] != 'M' || command[2] != '_') return false;

    arduinoNum = command[1] - '0';  // 아두이노 번호 추출
    action = command.substr(3);     // 명령어 추출

    if (action[0] == 'D') {  // 깜빡임 명령어일 경우
        delaySeconds = std::stoi(action.substr(1));
    }
    return true;
}

bool readLEDStatus(int file) {
    unsigned char buffer[1];
    buffer[0] = READ_REGISTER;
    
    if (write(file, buffer, 1) != 1) {
        std::cerr << "데이터 전송에 실패했습니다." << std::endl;
        return false;
    }

    unsigned char status;
    if (read(file, &status, 1) != 1) {
        std::cerr << "데이터 수신에 실패했습니다." << std::endl;
        return false;
    }

    std::cout << "LED 상태: " << (status == LED_ON ? "켜짐" : "꺼짐") << std::endl;
    return true;
}

int main() {
    // 시그널 핸들러 등록
    signal(SIGINT, cleanup);

    std::string command;
    int address;
    int arduinoNum, delaySeconds;
    std::string action;

    std::cout << "M?_ON, M?_OFF, M?_D??으로 명령을 줄 수 있습니다.\n?는 아두이노 번호이고, ??는 깜빡이 딜레이(초)입니다." << std::endl;

    while (true) {
        std::cout << "명령을 입력하세요: ";
        std::cin >> command;

        // 명령어 파싱
        if (!parseCommand(command, arduinoNum, action, delaySeconds)) {
            std::cerr << "잘못된 명령 형식입니다." << std::endl;
            continue;
        }

        // 아두이노 주소 설정
        if(arduinoNum == 1){
            address = SLAVE_ADDRESS_1;
            std::cout << "첫 번째 아두이노를 선택했습니다." << std::endl;
        }else if(arduinoNum == 2){
            address = SLAVE_ADDRESS_2;
            std::cout << "두 번째 아두이노를 선택했습니다." << std::endl;
        }else{
            std::cerr << "잘못된 아두이노 번호입니다." << std::endl;
            continue;
        }

        file = openI2CDevice(address);
        if (file < 0) continue;

        // 명령에 따라 LED 제어
        if (action == "ON") {
            controlLED(file, true);
        } else if (action == "OFF") {
            controlLED(file, false);
        } else if (action[0] == 'D') {
            blinkLED(file, delaySeconds);
        } else {
            std::cerr << "알 수 없는 명령입니다." << std::endl;
        }

        close(file);  // I2C 장치 닫기
    }

    cleanup(0);  // 프로그램 종료 시 클린업 함수 호출
    return 0;
}
