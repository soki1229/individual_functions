int TTS_MessageSpeak::dispatchMessage(MESSAGE_QUEUE_DATA * msgDATA)
{
    TTS_Manager *pTTS_Manager = TTS_Manager::getInstance();

    switch (msgDATA->ID) {
    case TTS_MSGID_CONFIGURE:
    {
        switch (msgDATA->PARAM) {
        case TTS_PARAM_CONFIGURE_DEFAULT:
        {
            pTTS_Manager->initialize();

            break;
        }
        case TTS_PARAM_CONFIGURE_LANGUAGE:
        {
            pTTS_Manager->setLanguage(msgDATA->STR_DATA);

            break;
        }
        case TTS_PARAM_CONFIGURE_VOICETYPE:
        {
            pTTS_Manager->setSpeaker(msgDATA->VALUE);

            break;
        }
        default:
            break;
        }

        break;
    }
    case TTS_MSGID_SYNTHESIS:
    {
        switch (msgDATA->PARAM) {
        case TTS_PARAM_SYNTHESIS_START:
        {
            pTTS_Manager->startSynthesizer(msgDATA->VALUE, msgDATA->STR_DATA);

            break;
        }
        default:
            break;
        }

        break;
    }
    default:
        break;
    }

    return RET_ERROR_STATUS_NONE;
}
