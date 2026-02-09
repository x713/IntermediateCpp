#pragma once

#include "../../../core/cli/ComTypes.h"
#include "../../../core/cli/CLIState.h"


#include "../../../util/Utils.h"
#include <string>
#include <thread>
#include <functional>

#include <iostream>
#include <boost/asio.hpp>

using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

//using lab::worker::IPCServerJob;
//using lab::worker::IPCClientJob;

using lab::util::Utils;


// 1. Boost timer usage
CLIState::State BostTimerCommand001(CLI_Vector args)
{
  Utils::Log("BostTimerCommand001");

  boost::asio::io_context io;
  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
  t.wait();

  Utils::Log("BostTimerCommand001 - timer done");

  return CLIState::State::OK;
}

// Timer.2 - Using a timer asynchronously
void print(const boost::system::error_code& /*e*/)
{
  std::cout << "BostTimerCommand002 - print" << std::endl;
}

// Timer.2 - Using a timer asynchronously
CLIState::State BostTimerCommand002(CLI_Vector args)
{
  Utils::Log("BostTimerCommand002");

  boost::asio::io_context io;

  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));

  t.async_wait(&print);

  io.run();

  Utils::Log("BostTimerCommand002 - timer done");

  return CLIState::State::OK;
}


// Timer.3 - Binding arguments to a completion handler
void print_binded(const boost::system::error_code& /*e*/,
  boost::asio::steady_timer* t, int* count)
{
  if (*count < 5)
  {
    std::cout << *count << std::endl;
    ++(*count);

    t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
    t->async_wait(std::bind(print_binded,
      boost::asio::placeholders::error, t, count));
  }
}

CLIState::State BostTimerCommand003(CLI_Vector args)
{
  Utils::Log("BostTimerCommand003");

  boost::asio::io_context io;

  int count = 0;
  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));

  t.async_wait(std::bind(print_binded,
    boost::asio::placeholders::error, &t, &count));

  io.run();

  Utils::Log("Final count is " + std::to_string(count));
  Utils::Log("BostTimerCommand003 - timer done");

  return CLIState::State::OK;
}
