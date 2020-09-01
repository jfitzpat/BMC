/*
    Frame.h
    Editable Frame Container
 
    Copyright 2020 Scrootch.me!

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <JuceHeader.h>

class Frame
{
public:
    Frame();
    ~Frame();
    
    File getImageFile() { return imageFile; }
    void setImageFile (const File& newFile) { imageFile = newFile; }
    const Image* getBackgroundImage() { return backgroundImage.get(); }
    void setOwnedBackgroundImage (Image* image) { backgroundImage.reset (image); }
    
    float getImageScale()               { return imageScale; }
    void setImageScale (float scale)    { imageScale = scale; }
    float getImageRotation()            { return imageRotation; }
    void setImageRotation (float rot)   { imageRotation = rot; }
    float getImageXoffset()             { return imageXoffset; }
    void setImageXoffset (float off)    { imageXoffset = off; }
    float getImageYoffset()             { return imageYoffset; }
    void setImageYoffset (float off)    { imageYoffset = off; }

private:
    File imageFile;
    std::unique_ptr<Image> backgroundImage;
    float imageScale;
    float imageRotation;
    float imageXoffset;
    float imageYoffset;
};
