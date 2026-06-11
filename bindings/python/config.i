%module(package="pyspiceql") config

%{
  #include <SpiceQL/config.h>
%}

%include <SpiceQL/config.h>

%extend SpiceQL::Config {
    SpiceQL::Config __getitem__(std::string ptr) {
        return (*($self))[ptr];
    }

    SpiceQL::Config __getitem__(std::vector<std::string> ptr) {
        return (*($self))[ptr];
    }
}
