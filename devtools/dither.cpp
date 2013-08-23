#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <QtGui/QImage>

#include <imageviewer/floyd_steinberg_grayscale.h>

int main(int argc, char* argv[]) {
    using namespace std;
    namespace po = boost::program_options;

    po::options_description desc("Options");
    desc.add_options()
            ("help", "produce help message")
            ("level", po::value<unsigned int>()->default_value(16),
             "set compression level")
            ("output,o", po::value<std::string>(), "Output filename")
            ("input", po::value<std::string>(), "Input filename")
            ;

    po::positional_options_description pos;
    pos.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") ||
        vm.count("output") == 0 ||
        vm.count("input") == 0) {
        cout << desc << endl;
        return 1;
    }

    QImage image(vm["input"].as<std::string>().c_str());
    image::FloydSteinbergGrayscale
            dithering_strategy(vm["level"].as<unsigned int>());
    dithering_strategy.dither(&image);
    image.save(vm["output"].as<std::string>().c_str());
    return 0;
}
