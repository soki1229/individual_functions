int TTS_Manager::startSynthesizer(int codeKind, std::string promptText)
{
    TTS_Service *pTTS_Service   = TTS_Service::getInstance();
    BaseUtil    *pBaseUtil      = BaseUtil::getInstance();

    LoggerService::getServices()->Log(_LOG_INFO, "TTS_Manager", _S_S_PAIR3 , __FUNCTION__, promptText);

    pBaseUtil->synthesizer()->interruptSynthesis = false;

    (void)pBaseUtil->synthesizer()->ComponentResourceLoad();

    if (pBaseUtil->synthesizer()->ComponentExecute(codeKind, promptText) != RET_TTS_SYNTHESIZE_STOP)
    {
        pTTS_Service->notifySpeechStatus(HSpeechStatusType::FINISHED);
    }

    (void)pBaseUtil->synthesizer()->ComponentResourceUnload();

    pTTS_Service->setServiceStatus(ServiceStatusType::IDLE);

    return RET_ERROR_STATUS_NONE;
}

int TTS_Manager::stopSynthesizer()
{
    BaseUtil *pBaseUtil = BaseUtil::getInstance();

    (void)pBaseUtil->synthesizer()->ComponentStop();

    return 0;
}
