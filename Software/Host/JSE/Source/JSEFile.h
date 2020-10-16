/*
  ==============================================================================

    JSEFile.h
    Created: 12 Sep 2020 9:49:07am
    Author:  Joseph Fitzpatrick

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define JSE_FILE_VERSION (1)

namespace JSEFile
{
    const Identifier AppVersion     ("JseVersion");
    const Identifier FileVersion    ("JseFileVersion");
    const Identifier FrameCount     ("FrameCount");
    const Identifier Frames         ("Frames");
    const Identifier ImageFileSize  ("ImageFileSize");
    const Identifier ImageFile      ("ImageFile");
    const Identifier ImageOpacity   ("ImageOpacity");
    const Identifier ImageScale     ("ImageScale");
    const Identifier ImageRotation  ("ImageRotation");
    const Identifier ImageXOffset   ("ImageXOffset");
    const Identifier ImageYOffset   ("ImageYOffset");
    const Identifier PointCount     ("PointCount");
    const Identifier Points         ("Points");
    const Identifier PointX         ("x");
    const Identifier PointY         ("y");
    const Identifier PointZ         ("z");
    const Identifier PointRed       ("r");
    const Identifier PointGreen     ("g");
    const Identifier PointBlue      ("b");
    const Identifier PointStatus    ("s");
    const Identifier iPaths         ("iPaths");
    const Identifier iPathRed       ("R");
    const Identifier iPathGreen     ("G");
    const Identifier iPathBlue      ("B");
    const Identifier iPathDensity   ("PointDensity");
    const Identifier ExtraPerAnchor ("ExtraPerAnchor");
    const Identifier BlanksBefore   ("BlanksBefore");
    const Identifier BlanksAfter    ("BlanksAfter");
    const Identifier Anchors        ("Anchors");
    const Identifier AnchorX        ("X");
    const Identifier AnchorY        ("Y");
    const Identifier AnchorEntryX   ("enX");
    const Identifier AnchorEntryY   ("enY");
    const Identifier AnchorExitX    ("exX");
    const Identifier AnchorExitY    ("exY");
}
