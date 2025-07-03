#pragma once

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "vm/Logger.hpp"

namespace vm {

// Redirects stdout to capture console output for GUI display
class ConsoleCapture {
public:
    explicit ConsoleCapture(BufferedLogger* logger = nullptr) 
        : m_logger(logger), m_oldBuf(std::cout.rdbuf()) {
        std::cout.rdbuf(m_buffer.rdbuf());
    }
    
    ~ConsoleCapture() {
        std::cout.rdbuf(m_oldBuf);
    }
    
    std::string getAndClear() {
        std::string result = m_buffer.str();
        m_buffer.str("");
        m_buffer.clear();
        
        // Forward to logger if available
        if (m_logger && !result.empty()) {
            // Split by lines and add each
            std::istringstream iss(result);
            std::string line;
            while (std::getline(iss, line)) {
                m_logger->info("[OUT] " + line);
            }
        }
        
        return result;
    }
    
private:
    BufferedLogger* m_logger{nullptr};
    std::ostringstream m_buffer;
    std::streambuf* m_oldBuf{nullptr};
};

} // namespace vm
