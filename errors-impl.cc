module errors;

import <string>;

using namespace std;

RaiiError::~RaiiError() = default;

// what forwards to message so catch blocks using std::exception can see the message
const char *RaiiError::what() const noexcept
{
    return message();
}

ParseError::ParseError(const string &m) noexcept
    : msg{m} {}

ParseError::~ParseError() = default;

const char *ParseError::message() const noexcept
{
    return msg.c_str();
}

MoveError::MoveError(const string &m) noexcept
    : msg{m} {}

MoveError::~MoveError() = default;

const char *MoveError::message() const noexcept
{
    return msg.c_str();
}

AbilityError::AbilityError(const string &m) noexcept
    : msg{m} {}

AbilityError::~AbilityError() = default;

const char *AbilityError::message() const noexcept
{
    return msg.c_str();
}

FatalError::FatalError(const string &m) noexcept
    : msg{m} {}

FatalError::~FatalError() = default;

const char *FatalError::message() const noexcept
{
    return msg.c_str();
}
