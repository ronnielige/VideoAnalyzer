#ifndef _PLAY_H_
#define _PLAY_H_

#include "Form1.h"

using namespace VideoAnalyzer;

System::Void readThreadProc(Object^ data);
System::Void decodeThreadProc(Object^ data);
System::Void renderThreadProc(Object^ data);

#endif