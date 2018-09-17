

#include "server.h"


std::unique_ptr<DNSServer> application;
BOOL WINAPI CtrlHandler(DWORD EventType);



int main(int argc, char* argv[]) {

	SetConsoleCtrlHandler(CtrlHandler, true); // Минутка не кросслатформенного кода		

	try {

		application = std::make_unique<DNSServer>(DNSServerCreateInfo::CreateFromFile("config.json"));
		application->Run();
	}
	catch (std::exception const& e) {
		application->Shutdown();
		std::cout << "Error: " << e.what();
	}

}


BOOL WINAPI CtrlHandler(DWORD EventType) {
	application->Shutdown();
	switch (EventType) {
		case CTRL_C_EVENT:
			return(TRUE);
		case CTRL_CLOSE_EVENT:
			return(TRUE);
		case CTRL_BREAK_EVENT:
			return FALSE;
		case CTRL_LOGOFF_EVENT:
			return FALSE;
		case CTRL_SHUTDOWN_EVENT:
			return FALSE;
		default:
			return FALSE;
	}
}

