#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "session.h"
#include "response.h"
#include <string>

class dispatcher {
  public:
    dispatcher(session* session);

    /* Parse a response object into string */
    std::string ToString(const Response& response);

    /* dispatch */
    void dispatch(std::string response);

  private:
    session* session_:
};

#endif
