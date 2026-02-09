#include <array>
#include <boost/asio.hpp>
#include <iostream>


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

using boost::asio::ip::tcp;



// Daytime.1 - A synchronous TCP daytime client

CLIState::State DatimeCommand01(CLI_Vector args)
{
  Utils::Log("Datime01");

  boost::asio::io_context io;
  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
  t.wait();

  Utils::Log("Datime01 - timer done");

  return CLIState::State::OK;
}