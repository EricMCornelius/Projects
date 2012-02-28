/*
 * RequestProcessor.h
 *
 *  Created on: Feb 25, 2012
 *      Author: corneliu
 */

#ifndef REQUESTPROCESSOR_H_
#define REQUESTPROCESSOR_H_

#include "ReadBuffer.h"
#include "WriteBuffer.h"

#include <iostream>

template <typename ReadBuffer, typename WriteBuffer>
class RequestProcessor {
public:
  RequestProcessor()
  {
    std::string testMsg =
            "HTTP/1.0 200 OK\r\n"
            "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 11\r\n"
            "\r\n"
            "Hello World";

    std::copy(std::begin(testMsg), std::end(testMsg), std::begin(_output));
    std::cout << testMsg.size() << " " << _output.size() << std::endl;
  }

  void reset() {
    _input.reset();
    _output.reset();
  }

  void advanceInput(size_t len) {
    _input.advance(len);
    processInput();
  }

  void processInput() {
    size_t read = _input.size();
    std::cout << read << std::endl;
    if (_input.size() > 3) {
      if (_input[read - 4] == '\r' && _input[read - 3] == '\n' && _input[read - 2] == '\r' && _input[read - 1] == '\n') {
        for (size_t i = 0; i < read; ++i)
          std::cout << _input[i];
        std::cout << std::endl;
        writeCb();
      }
    }
  }

  void advanceOutput(size_t len) {
    _output.advance(len);
    /*
    std::cout << len << " " << _written << std::endl;
    for (size_t i = 0; i < _written; ++i)
      std::cout << (*this)[i];
    std::cout << std::endl;
    return true;
    */
  }

  ReadBuffer& inputBuffer() {
    return _input;
  }

  WriteBuffer& outputBuffer() {
    return _output;
  }

  typedef std::function<void()> SignalWriteCb;

  SignalWriteCb writeCb;

private:
  ReadBuffer _input;
  WriteBuffer _output;
};


#endif /* REQUESTPROCESSOR_H_ */
