#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <fstream>
#include <string>

void runCommand(const std::string& command) {
    std::string fullCommand = "powershell -Command \"" + command + "\"";
    system(fullCommand.c_str());
}

std::string checkDefenderStatus() {
    runCommand("Get-MpComputerStatus | Select-Object -ExpandProperty RealTimeProtectionEnabled > defender_status.txt");
    std::ifstream statusFile("defender_status.txt");
    std::string status;
    if (statusFile.is_open()) {
        std::getline(statusFile, status);
        statusFile.close();
    }
    remove("defender_status.txt");
    if (status == "True") {
        return "Microsoft Defender is running (Green)";
    }
    else {
        return "Microsoft Defender is turned off (Red)";
    }
}

void disableTamperProtection() {
    runCommand("Start-Process powershell -ArgumentList 'Set-MpPreference -DisableTamperProtection $true' -Verb RunAs");
}

void enableTamperProtection() {
    runCommand("Start-Process powershell -ArgumentList 'Set-MpPreference -DisableTamperProtection $false' -Verb RunAs");
}

void disableDefender() {
    disableTamperProtection();

    runCommand("Set-MpPreference -DisableRealtimeMonitoring $true");
    runCommand("sc.exe config WinDefend start=disabled");
    runCommand("sc.exe stop WinDefend");

    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Scheduled Scan\" /DISABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Cache Maintenance\" /DISABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Cleanup\" /DISABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Verification\" /DISABLE");

    std::cout << "Microsoft Defender has been disabled (Red)" << std::endl;
}

void enableDefender() {
    runCommand("sc.exe config WinDefend start=auto");
    runCommand("sc.exe start WinDefend");

    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Scheduled Scan\" /ENABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Cache Maintenance\" /ENABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Cleanup\" /ENABLE");
    runCommand("schtasks.exe /Change /TN \"Microsoft\\Windows\\Windows Defender\\Windows Defender Verification\" /ENABLE");

    runCommand("Set-MpPreference -DisableRealtimeMonitoring $false");
    enableTamperProtection();

    std::cout << "Microsoft Defender has been enabled (Green)" << std::endl;
}

void addToExclusionList() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string scriptPath = std::string(path);
    runCommand("Add-MpPreference -ExclusionPath '" + scriptPath + "'");
    std::cout << "Added " << scriptPath << " to Microsoft Defender exclusion list" << std::endl;
}

int main() {
    int choice;

    do {
        std::cout << "\nDefender Control Program\n";
        std::cout << "1. Check Microsoft Defender Status\n";
        std::cout << "2. Disable Microsoft Defender Permanently\n";
        std::cout << "3. Enable Microsoft Defender Permanently\n";
        std::cout << "4. Add Program to Defender Exclusion List\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << checkDefenderStatus() << std::endl;
            break;
        case 2:
            disableDefender();
            break;
        case 3:
            enableDefender();
            break;
        case 4:
            addToExclusionList();
            break;
        case 5:
            std::cout << "Exiting..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice, please try again." << std::endl;
        }
    } while (choice != 5);

    return 0;
}
