/* File : JavaScript_sml_ClientInterface.i */
%module JavaScript_sml_ClientInterface

// handle windows calling convention, __declspec(dllimport), correctly
%include <windows.i>

// Reuse shared SML interface definitions
%include "../sml_ClientInterface.i"
