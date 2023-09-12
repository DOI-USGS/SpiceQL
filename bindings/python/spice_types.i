%module(package="pyspiceql") spice_types

%{
  #include "spice_types.h"
%}

%rename(NonMemo_translateNameToCode) SpiceQL::translateNameToCode;
%rename(NonMemo_translateCodeToName) SpiceQL::translateCodeToName;

%include "spice_types.h"
