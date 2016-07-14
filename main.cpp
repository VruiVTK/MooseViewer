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
    std::cout << "\t-r <digit>, -renderMode <digit>" << std::endl;
    std::cout << "\tRender mode to request for vtkSmartVolumeMapper.\n" << std::endl;
    std::cout << "\t-showfps" << std::endl;
    std::cout << "\tShow the FPS display by default.\n" << std::endl;
    std::cout << "\t-benchmark" << std::endl;
    std::cout << "\tPrints timing information for data updates to stderr.\n" << std::endl;
    std::cout << "\t-hidebgnotifs" << std::endl;
    std::cout << "\tHide notifications for background updates.\n" << std::endl;
    std::cout << "\t-widgetHints <path>" << std::endl;
    std::cout << "\tPath to a JSON file providing widget hints.\n" << std::endl;
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
    int renderMode = -1;
    bool showFPS = false;
    bool benchmark = false;
    bool hidebgnotifs = false;
    std::string widgetHints;
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
        if(strcmp(argv[i], "-r")==0 || strcmp(argv[i], "-renderMode")==0)
          {
          renderMode = atoi(argv[i+1]);
          ++i;
          }
        if(strcmp(argv[i], "-showfps")==0)
          {
          showFPS = true;
          }
        if(strcmp(argv[i], "-benchmark")==0)
          {
          benchmark = true;
          }
        if(strcmp(argv[i], "-hidebgnotifs")==0)
          {
          hidebgnotifs = true;
          }
        if(strcmp(argv[i], "-widgetHints")==0)
          {
          widgetHints.assign(argv[i+1]);
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
    application.setShowFPS(showFPS);
    application.setBenchmark(benchmark);
    application.setProgressVisibility(!hidebgnotifs);
    application.setWidgetHintsFile(widgetHints);
    if(!name.empty())
      {
      application.setFileName(name.c_str());
      }
    if(renderMode != -1)
      {
      application.setRequestedRenderMode(renderMode);
      }
    application.initialize();
    application.run();
    return 0;
    }
  catch (std::runtime_error e)
    {
    std::cerr << "Error: Exception " << e.what() << "!" << std::endl;
    return 1;
    }
}
