//
// Created by yura on 3/15/25.
//

#ifndef TEST_TASK_EXCEPTIONS_H
#define TEST_TASK_EXCEPTIONS_H

#include <exception>

class ProcessingException : public std::exception {
public:
    explicit ProcessingException(const char *message_) : message(message_) {}

    [[nodiscard]] const char *what() const noexcept override {
        return message;
    }

private:
    const char *message;
};
#endif //TEST_TASK_EXCEPTIONS_H
