#include "logger.h"

Can::Logger* Can::GlobalLogger::instance(){
    return Can::GlobalLogger::m_logger;
}
#ifdef __MINGW32__
std::string appdata = getenv("APPDATA");
Can::Logger* Can::GlobalLogger::m_logger = new Can::FileLogger(appdata + "\\canFlash\\canFlash.log");
// Can::Logger* Can::GlobalLogger::m_logger = new Can::NoLogger();
#else
Can::Logger* Can::GlobalLogger::m_logger = new Can::FileLogger("/tmp/canFlash.log");
// Can::Logger* Can::GlobalLogger::m_logger = new Can::NoLogger();
#endif
