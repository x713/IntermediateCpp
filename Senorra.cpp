// Senorra.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/*
#include <iostream>
#include <string>


#include <thread>

#include    "CLIProcessor.h"
#include    "ComTypes.h"
#include    "Utils.h"


using lab::util::Utils;


using lab::cli::CLI_Vector;
using lab::cli::CLIState;
using lab::cli::CLIState_toInt;
using lab::cli::CLIProcessor;
using lab::cli::CLICommandTestFAIL;
using lab::cli::CLICommandTestPASS;
 */


#include    "CLIProcessor.h"
#include    "MainCommand.h"

using lab::cli::CLIProcessor;
using lab::cli::CLIState;

int main(const int argc, const char* argv[])
{

  CLIProcessor cmdProc(argc, argv);
  cmdProc.TrimProgramName(argv[0]);

  cmdProc.AddCommand("cp", MainCommand);
  //cmdProc.AddCommand("ncp", NetworkCommand);

  int result = CLIState::toInt(cmdProc.Run());

  return result;
}

