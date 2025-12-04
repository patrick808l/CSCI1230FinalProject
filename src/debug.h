#pragma once
#include <GL/glew.h>
#include <iostream>

namespace Debug
{
// Task 2: Add file name and line number parameters
inline void glErrorCheck(const char* fileName, int lineNumber) {
    GLenum errorNumber = glGetError();
    while (errorNumber != GL_NO_ERROR) {
        std::cerr << "Error at " << fileName << ":" << lineNumber << ": " << errorNumber << std::endl;

        errorNumber = glGetError();
    }
}
}

#define glErrorCheck() glErrorCheck(__FILE__, __LINE__)
