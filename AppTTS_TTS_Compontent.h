#ifndef TTS_COMPONENT_H
#define TTS_COMPONENT_H

#include "BaseDefinition.h"
#include "LoggerService.h"

class TTS_Component
{
public:
    TTS_Component() = default;
    virtual ~TTS_Component() = default;
    
    virtual unsigned int ComponentOpen() = 0;
    virtual unsigned int ComponentClose() = 0;
    virtual unsigned int ComponentSetParameter() = 0;
    virtual unsigned int ComponentResetParameters() = 0;
    virtual unsigned int ComponentSetSpeaker() = 0;
    virtual unsigned int ComponentResourceLoad() = 0;
    virtual unsigned int ComponentResourceUnload() = 0;
    virtual unsigned int ComponentExecute(int codeKind, std::string text) = 0;
    virtual unsigned int ComponentStop() = 0;
    
    TTS_INFO    *pConfig            = nullptr;
    
    int         nSpeaker            = -1;
    
    bool        interruptSynthesis  = false;
    
private:
    
};

#endif // TTS_COMPONENT_H
