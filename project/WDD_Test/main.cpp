#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "../WDD_UDP/HymWdd.h"
using namespace std;

int main()
{
	uint32_t LibVer;
	int rtn = 2;
    unsigned char data[1500] = { 0 };
	gmc_get_lib_version(&LibVer);
	std::cout << "LIB��汾��: " << std::hex << LibVer << std::endl;

	// �����豸
	rtn = ConnectedDevice();
	std::cout << rtn << std::endl;

	// ���ò���
	rtn = ConfigureADCParameters(0, 100000, 0);
	std::cout << rtn << std::endl;

	// ��ʼ�ɼ�
	rtn = StartADCCollection();
	std::cout << rtn << std::endl;

	// �������̲߳��˳�
	std::string userInput;
	std::cout << "Enter 'exit' to stop: " << std::endl;
	while (std::cin >> userInput) {
		if (userInput == "exit") {
			break;
		}
		std::cout << "Invalid input. Enter 'exit' to stop: " << std::endl;
	}

	// ��ȡ����
	uint32_t Length = 1024;
	rtn = TryReadADCData(data, Length);
	std::cout << "Read buffer data: ";
	for (int i = 0; i < Length; ++i) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(static_cast<unsigned char>(data[i]))) << " ";
	}
	std::cout << std::endl;
	std::cout << "Return value: " << rtn << std::endl;

	// ֹͣ�ɼ�
	rtn = StopADCCollection();
	std::cout << rtn << std::endl;

	// �ر��豸
	rtn = CloseDevice();
	std::cout << rtn << std::endl;
	return 0;
}

