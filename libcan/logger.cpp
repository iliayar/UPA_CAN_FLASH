#include "logger.h"

Can::Logger* Can::GlobalLogger::instance(){
    return Can::GlobalLogger::m_logger;
}
#ifdef __MINGW32__
std::string appdata = getenv("APPDATA");
Can::Logger* Can::GlobalLogger::m_logger = new Can::FileLogger(appdata + "\\canFlash\\canFlash.log");
#else
Can::Logger* Can::GlobalLogger::m_logger = new Can::FileLogger(".local/canFlash.log");
#endif