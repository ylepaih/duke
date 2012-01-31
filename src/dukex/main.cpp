#include "Configuration.h"
#include <dukexgui/UIApplication.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <stdexcept>

namespace { // empty namespace
void terminateFunc(void) {
    std::cerr << "Sorry, the application has encountered a fatal error." << std::endl;
    std::cerr << "Please report this bug." << std::endl;
}
void unexpectedFunc(void) {
    throw std::runtime_error("Sorry, the application has encountered an unexpected exception.\nPlease report this bug.");
}
} // empty namespace


int main(int argc, char *argv[]) {
    std::set_terminate(&terminateFunc);
    std::set_unexpected(&unexpectedFunc);
    try {

        std::cout << "DukeX Player" << std::endl;

        // Qt resources & CSS
        Q_INIT_RESOURCE(resources);
        // NOTE: GraphicsSystem "raster" is needed on unix systems to
        //       fix huge QML performance issue.
        QApplication::setGraphicsSystem("raster");
        QApplication qapp(argc, argv);
        qapp.setStyle(new QCleanlooksStyle());
        QFile file(":/CSS/dark.css");
        if (file.open(QIODevice::ReadOnly)) {
            QString styleSheet = QLatin1String(file.readAll());
            qapp.setStyleSheet(styleSheet);
        }
        // Configuration - command line options
        Configuration conf(argc, argv);
        if (conf.stopped())
            return EXIT_SUCCESS;

        // Run our main Qt Application
        UIApplication uiapp(conf.path(), conf.port());
        uiapp.show();

        return qapp.exec();
    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }
}

