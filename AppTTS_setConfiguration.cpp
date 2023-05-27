unsigned int BaseUtil::setConfiguration(TTS_CONFIGURATION configuration)
{
    switch (configuration) {
    case TTS_CONFIGURATION::TTS_ENGINE:
    {
        if (getAreaCode().empty())
        {
            LoggerService::getServices()->Log(_LOG_DEBUG, __FUNCTION__, "AREA CODE WAS NOT PRE-CONFIGURED");

            return RET_ERROR_STATUS_FAIL;
        }
        else if (getAreaCode().compare("USA") == 0)
        {
            this->pSynthesizer = Component_Houndify::getInstance();
        }
        else if (getAreaCode().compare("KOR") == 0)
        {
            this->pSynthesizer = Component_ReadSpeaker::getInstance();
        }
        else if (getAreaCode().compare("EUR") == 0)
        {
            this->pSynthesizer = Component_Vocalizer::getInstance();
        }
        else {
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "SET DEFAULT ENGINE FOR AREA(%s)", getAreaCode());

            this->pSynthesizer = Component_Vocalizer::getInstance();
        }

        LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "########## <%s> engine set for <%s> region ##########", getEngineName(), getAreaCode());

        break;
    }
    case TTS_CONFIGURATION::TTS_LANGUAGE:
    {
        if (this->synthesizer() == nullptr)
        {
            LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "TTS Engine is not pre-defined.");
            break;
        }

        if (this->synthesizer()->pConfig != nullptr)
        {
            LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "Language Changed, release previous configuration");

            (void)this->synthesizer()->ComponentResourceUnload();

            (void)this->synthesizer()->ComponentClose();
        }

        auto iter = this->dataMap.find(currentLanguageData()->name_short);
        if (iter != this->dataMap.end())
        {
            this->synthesizer()->pConfig = &iter->second;

            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, ">> %s", iter->second.engineCode);
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, ">> %ld", iter->second.samplingFrequency);
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, ">> %ld", iter->second.frameBufferSize);
            for (auto&& elem : iter->second.resourcePaths)
            {
                LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, ">> %s", elem);
            }
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "  -->> %s", iter->second.languageCode);
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "  -->> %s", iter->second.voiceName);
            LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "  -->> %s", iter->second.modelName);
            for (auto&& elem : iter->second.tunedResources)
            {
                LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "  -->> %s", elem);
            }
            (void)this->synthesizer()->ComponentOpen();

            (void)this->synthesizer()->ComponentSetParameter();
        }

    } //continue
    case TTS_CONFIGURATION::TTS_SPEAKER:
    {
        if (this->synthesizer() == nullptr || this->synthesizer()->pConfig == nullptr)
        {
            LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "TTS Engine is not pre-defined.");
        }

        (void)this->synthesizer()->ComponentSetSpeaker();

        break;
    }
    default:
        break;
    }

    return RET_ERROR_STATUS_NONE;
}
