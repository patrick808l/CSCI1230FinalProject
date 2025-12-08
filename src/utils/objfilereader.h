#ifndef OBJFILEREADER_H
#define OBJFILEREADER_H

#include <memory>
#include <vector>
#include <string>
#include <GL/glew.h>

int readAndParseFile(std::string meshfile, std::shared_ptr<std::vector<GLfloat>> vertData);

#endif // OBJFILEREADER_H
