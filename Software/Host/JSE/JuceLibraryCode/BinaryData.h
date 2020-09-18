/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   croshair_png;
    const int            croshair_pngSize = 2173;

    extern const char*   downwhite_png;
    const int            downwhite_pngSize = 2538;

    extern const char*   duplicatewhite_png;
    const int            duplicatewhite_pngSize = 2727;

    extern const char*   pointinghand_png;
    const int            pointinghand_pngSize = 2438;

    extern const char*   showall_png;
    const int            showall_pngSize = 2762;

    extern const char*   upwhite_png;
    const int            upwhite_pngSize = 2501;

    extern const char*   zoomin_png;
    const int            zoomin_pngSize = 3369;

    extern const char*   zoomout_png;
    const int            zoomout_pngSize = 3300;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 8;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
