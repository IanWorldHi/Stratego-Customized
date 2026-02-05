export module errors;

import <exception>;
import <string>;

using namespace std;

// RaiiError is the abstract base for all game level errors
export class RaiiError : public exception
{
public:
    RaiiError() noexcept = default;
    ~RaiiError() override;

    // message returns a c string describing the error
    virtual const char *message() const noexcept = 0;

    // what forwards to message so the handlers can use it
    const char *what() const noexcept override;
};

// ParseError reports problems while reading input or options
export class ParseError : public RaiiError
{
    string msg;

public:
    explicit ParseError(const string &m) noexcept;
    ~ParseError() override;

    // message returns the stored error text
    const char *message() const noexcept override;
};

// MoveError reports invalid moves issued by the player
export class MoveError : public RaiiError
{
    string msg;

public:
    explicit MoveError(const string &m) noexcept;
    ~MoveError() override;

    // message returns the stored error text
    const char *message() const noexcept override;
};

// AbilityError reports invalid or unusable ability actions
export class AbilityError : public RaiiError
{
    string msg;

public:
    explicit AbilityError(const string &m) noexcept;
    ~AbilityError() override;

    // message returns the stored error text
    const char *message() const noexcept override;
};

// FatalError represents unrecoverable internal failures
export class FatalError : public RaiiError
{
    string msg;

public:
    explicit FatalError(const string &m) noexcept;
    ~FatalError() override;

    // message returns the stored error text
    const char *message() const noexcept override;
};
