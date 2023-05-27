unsigned int BaseUtil::setConfiguration(TTS_CONFIGURATION configuration)
{
	BaseUtil *pBaseUtil = BaseUtil::getInstance();
	switch (configuration) {
	case TTS_CONFIGURATION::TTS_ENGINE:
	{
	    std::string regionCode = pBaseUtil->getAreaCode();
	    std::string engineType = std::string();
	    if (regionCode.compare("KOR") == 0)
	    {
	        this->pSynthesizer = new Component_ReadSpeaker;
	        engineType = "ReadSpeaker";
	    }
	    else if (regionCode.compare("USA") == 0)
	    {
	        this->pSynthesizer = new Component_Houndify;
	        engineType = "Houndify";
	    }
	    else
	    {
	        this->pSynthesizer = new Component_Vocalizer;
	        engineType = "Vocalizer";
	    }
	    LoggerService::getServices()->Log(_LOG_INFO, __FUNCTION__, "########## <%s> engine set for <%s> region ##########", engineType, regionCode);
	    break;
	}
	case TTS_CONFIGURATION::TTS_LANGUAGE:
	{
	    if (this->pSynthesizer == nullptr)
	    {
	        LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "TTS Engine is not pre-defined.");
	        break;
	    }
	    if (this->pSynthesizer->pConfig != nullptr)
	    {
	        LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "Language Changed, release previous configuration");
	        (void)this->pSynthesizer->ComponentResourceUnload();
	        (void)this->pSynthesizer->ComponentClose();
	    }
	    auto iter = this->dataMap.find(pBaseUtil->currentLanguageData()->name_short);
	    if (iter != this->dataMap.end())
	    {
	        this->pSynthesizer->pConfig = &iter->second;
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
	        (void)this->pSynthesizer->ComponentOpen();
	        (void)this->pSynthesizer->ComponentSetParameter();
	    }
	} //continue
	case TTS_CONFIGURATION::TTS_SPEAKER:
	{
	    if (this->pSynthesizer == nullptr || this->pSynthesizer->pConfig == nullptr)
	    {
	        LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "TTS Engine is not pre-defined.");
	    }
	    (void)this->pSynthesizer->ComponentSetSpeaker();
	} // continue
	case TTS_CONFIGURATION::TTS_RESOURCES:
	{
	    if (this->pSynthesizer == nullptr || this->pSynthesizer->pConfig == nullptr)
	    {
	        LoggerService::getServices()->Log(_LOG_ERROR, __FUNCTION__, "TTS Engine is not pre-defined.");
	    }
	    (void)this->pSynthesizer->ComponentResourceLoad();
	    break;
	}
	default:
	    break;
	}
	return 0;
}
