Dialog gui application for windows. 
In principle, the complete dialog can be created from windows sdk calls in the source code.
For convenience, the sdk provides a function to build a complete dialog from a predefined 
DLGTEMPLATE structure. This structure can be loaded in precompiled binary form from
a ressource, embedded within the executable.
The dialog is originally created with the vc6 ressource editor ad stored in a .rc file.
The .rc file can also be edited manually. NOTE: Whenediting the .rc file manually, the
object ids of the various elements must also be edited in the ressource.h file accordingly,
since those ids are used by the various sdk functions in the code to target thos elements.
The .rc file is compiled with the ressource compiler to a .res binary.
The .res binary is packed into the executable binary by the compiler/linker?

Visual studio express does not bring a ressource editor nor a ressource compiler.
However, the free windows sdk packages bring a ressource compiler, so a previously 
created or manually edited .rc file can be used.

To build the project from the command line, first call the ressource compiler:
rc  gui_ressource.rc
Include the rssource file into the compile/build step:
cl  gui_main.c gui_ressource.rc /link user32 <+ other sdk libs>

To enable the program to adopt to the 'Style' of the windows version, it must be linked 
with the ComCtl32.lib library, which is not included in the Visual Studio express version,
but is containted in the windows sdk. Also note the strange 'Manifest' that has to be
written into the binary to enable Styles.
