// STD includes
#include <iostream>
#include <string>

// MooseViewer includes
#include "MooseViewer.h"

void printUsage(bool longForm = true)
{
  if (longForm)
    {
    std::cout << "\nMooseViewer - Render VTK objects in the VRUI context" << std::endl;
    std::cout << "Example application that demonstrates loading of MOOSE" <<
                 "\nframework Exodus II files." << std::endl;
    std::cout << "\nUSAGE:\n\t./MooseViewer -f <string> [-h]" << std::endl;
    std::cout << "\nWhere:" << std::endl;
    std::cout << "\t-f <string>, -fileName <string>" << std::endl;
    std::cout << "\tName of ExodusII file to load using VTK.\n" << std::endl;
    std::cout << "\t-h, -help" << std::endl;
    std::cout << "\tDisplay this usage information and exit." << std::endl;
    std::cout << "\nAdditionally, all the commandline switches that VRUI " <<
      "accepts\ncan be passed to MooseViewer.\nFor example, -rootSection," <<
      " -vruiVerbose, -vruiHelp, etc.\n" << std::endl;
    }
  else
    {
    std::cout << "\nUSAGE:\n\t./MooseViewer -f <string> [-h]" << std::endl;
    }
}

/* Create and execute an application object: */
/*
 * main - The application main method.
 *
 * parameter argc - int
 * parameter argv - char**
 *
 */
int main(int argc, char* argv[])
{
  try
    {
    std::string name;
    if(argc > 1)
      {
      /* Parse the command-line arguments */
      for(int i = 1; i < argc; ++i)
        {
        if(strcmp(argv[i], "-f")==0 || strcmp(argv[i], "-filename")==0)
          {
          name.assign(argv[i+1]);
          ++i;
          }
        if(strcmp(argv[i],"-h")==0 || strcmp(argv[i], "-help")==0)
          {
          printUsage();
          return 0;
          }
        }
      }

    if(name.empty())
      {
      std::cerr << "\nERROR: FileName not provided." << std::endl;
      printUsage(false);
      return 1;
      }

    MooseViewer application(argc, argv);
    if(!name.empty())
      {
      application.setFileName(name.c_str());
      }
    application.run();
    return 0;
    }
  catch (std::runtime_error e)
    {
    std::cerr << "Error: Exception " << e.what() << "!" << std::endl;
    return 1;
    }
}
