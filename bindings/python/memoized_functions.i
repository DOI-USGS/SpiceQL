%module(package="pyspiceql") memoized_functions


%{
  #include <SpiceQL/memoized_functions.h>
%}

%rename(Memo_ls) SpiceQL::Memo::ls;
%rename(Memo_getTimeIntervals) SpiceQL::Memo::getTimeIntervals;
%rename(Memo_globTimeIntervals) SpiceQL::Memo::globTimeIntervals;
%rename(Memo_getPathsFromRegex) SpiceQL::Memo::getPathsFromRegex;

%include <SpiceQL/memoized_functions.h>
