#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "session.h"
#include "response.h"
#include <string>
#include <vector>

class dispatcher
{
public:
    dispatcher(session *session);
    std::string ToString(Response::StatusCode status);
    void dispatch(const Response &response);

private:
    session *session_;
};

#endif
