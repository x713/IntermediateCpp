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

//
//  Timer 4. Contextual printer
//


class contxtedprinter
{
public:
  contxtedprinter(boost::asio::io_context& io)
    : timer_(io, boost::asio::chrono::seconds(1)),
    count_(0)
  {
    timer_.async_wait(std::bind(&contxtedprinter::print, this));
  }

  ~contxtedprinter()
  {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void print()
  {
    if (count_ < 5)
    {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
      timer_.async_wait(std::bind(&contxtedprinter::print, this));
    }
  }

private:
  boost::asio::steady_timer timer_;
  int count_;
};

CLIState::State BostTimerCommand004(CLI_Vector args)
{
  Utils::Log("BostTimerCommand004");

  boost::asio::io_context io;
  contxtedprinter p(io);
  io.run();

  Utils::Log("BostTimerCommand004 - done");

  return CLIState::State::OK;
}

//
// 5. SyncedExecutor strand
//


class SyncedExecutorPrinter
{
public:
  SyncedExecutorPrinter(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io)),
    timer1_(io, boost::asio::chrono::seconds(1)),
    timer2_(io, boost::asio::chrono::seconds(1)),
    count_(0)
  {
    timer1_.async_wait(boost::asio::bind_executor(strand_,
      std::bind(&SyncedExecutorPrinter::print1, this)));

    timer2_.async_wait(boost::asio::bind_executor(strand_,
      std::bind(&SyncedExecutorPrinter::print2, this)));
  }

  ~SyncedExecutorPrinter()
  {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void print1()
  {
    if (count_ < 10)
    {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expiry() + boost::asio::chrono::seconds(1));

      timer1_.async_wait(boost::asio::bind_executor(strand_,
        std::bind(&SyncedExecutorPrinter::print1, this)));
    }
  }

  void print2()
  {
    if (count_ < 10)
    {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expiry() + boost::asio::chrono::seconds(1));

      timer2_.async_wait(boost::asio::bind_executor(strand_,
        std::bind(&SyncedExecutorPrinter::print2, this)));
    }
  }

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::steady_timer timer1_;
  boost::asio::steady_timer timer2_;
  int count_;
};



CLIState::State BostTimerCommand005(CLI_Vector args)
{
  Utils::Log("BostTimerCommand005");

  // creating new thread allows to have io.run in parallel
  boost::asio::io_context io;
  SyncedExecutorPrinter p(io);

  std::thread t([&] { io.run(); });

  io.run();
  t.join();

  Utils::Log("BostTimerCommand005 - done");

  return CLIState::State::OK;
}