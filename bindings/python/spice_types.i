%module(package="pyspiceql") spice_types

%{
  #include <SpiceQL/spice_types.h>
%}


%include <SpiceQL/spice_types.h>


%extend SpiceQL::KernelSet {
  %pythoncode %{
  def __enter__(self):
      return self
  
  def __exit__(self, exc_type, exc_val, exc_tb):
      self.unload()
      return False
  %}
}
