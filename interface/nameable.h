#ifndef INAMABLE_H
#define INAMABLE_H

#include <string>

class INamable
{
public:
    virtual const std::string &getName() const = 0;

    virtual ~INamable() = default;
};

#endif