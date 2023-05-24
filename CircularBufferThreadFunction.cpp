void *VRWakeupRingBufferThread::CircularBufferThreadFunction(void *param)
{
    mzBASExLOGxLOGGERxINTERFACE *piLogger = mzBasexLogxLoggerxInterface();

    VRWakeupRingBufferThread* pThis = reinterpret_cast<VRWakeupRingBufferThread*>(param);

    const int frameCount        = 7;
    const int audioBufferSize   = (frameCount*16) * 16 * sizeof(int16_t); // 3584 => 112ms * 16Khz * 2 byte : 112ms
    const int rawBufferSize     = audioBufferSize/frameCount * INPUT_CHANNEL_COUNTS;

    pThis->circularBuffer.setRingBuffer(400, rawBufferSize);

    unsigned char *rBuff    = (unsigned char *)malloc(rawBufferSize);
    unsigned char *micBuff  = (unsigned char *)malloc(audioBufferSize * INPUT_CHANNEL_COUNTS);

    HAudioStreamIn *pAudioStreamIn = gAudioStreamIn;
    gAudioStreamIn = nullptr;

    if (pAudioStreamIn != nullptr)
    {
        wuwLogFunction("VRWakeupRingBufferThread::Loop_Func gAudioStreamIn is not nullptr -> error. (wait 1ms)");

        delete pAudioStreamIn;
        pAudioStreamIn = nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    pAudioStreamIn = new HAudioStreamIn;
    CAudioInDevice::initializeAudioStreamIn(pAudioStreamIn, HAudioChannel::CH_STEREO, HAudioStreamType::MAIN_IN, 16000, 256, 2048, 0);

    gAudioStreamIn = pAudioStreamIn;

    if (nullptr != pThis)
    {
        pThis->terminateThread = false;

        bool triggerBuff = false;

        int currentFrame    = 0;
        int nullBuffCount   = 0;

        while (pThis->terminateThread != true)
        {
            pAudioStreamIn->read(rBuff, 256);

            // VR NOT TRIGGERED YET
            if (pThis->isCapturingSpeechAudio() == false)
            {
                pThis->circularBuffer.push((char *)rBuff);

                triggerBuff = false;
                nullBuffCount = 0;
                pThis->interruptQueue = false;
            }
            // VR TRIGGERED
            else {
                // VR TRIGGERED BY WAKEUP
                if (pThis->isWakeUpDetected() == true && triggerBuff == false)
                {
                    wuwLogFunction(">> VR WAKEUP - PUSHING < 1. TRIGGER >");

                    int audioBufSize = 20 * audioBufferSize;  // 2.24 sec = (20 x 112ms) = 20 x (7 x 16ms)

                    char *AudioBuf = pThis->circularBuffer.getMemoryBuffer(&audioBufSize, pThis->getDetectionData()->spottedChannel);
                    if (AudioBuf != nullptr)
                    {
                        size_t nRemainSize = audioBufSize;
                        const char * pSrc = AudioBuf;

                        while (nRemainSize > 0)
                        {
                            size_t bufSize = (nRemainSize > audioBufferSize) ? audioBufferSize : nRemainSize;
                            char *pbBuffer = static_cast<char *>(malloc(audioBufferSize));
                            if (pbBuffer != nullptr) {
                                if(bufSize != audioBufferSize) { memset(pbBuffer,0,audioBufferSize); }
                                memcpy(pbBuffer, pSrc, bufSize);
                                pSrc += bufSize;
                                nRemainSize -= bufSize;
                                gAudioInBlockingQueue.push(pbBuffer);
                            }
                        }

                        pThis->saveAudioDumpFile("/tmp/vr/engineer_wav_cache/wakeup_trigger.pcm", AudioBuf, audioBufSize);
                        delete[] AudioBuf;
                    }

                    pThis->circularBuffer.clear();

                    pThis->setWakeUpDetected(false);
                    triggerBuff = true;
                }

                memcpy(micBuff + currentFrame*rawBufferSize, rBuff, rawBufferSize);

                currentFrame++;

                if (currentFrame < 7)
                {
                    continue;
                }

                char *queueBuff = (char *)malloc(audioBufferSize);


                (void)piLogger->Output(&_hBasexErrorxDLF, ">> VR WAKEUP - Audio Input Channel[%d]", pThis->getDetectionData()->spottedChannel);

                for (int i = 0, j = (pThis->getDetectionData()->spottedChannel == 2)? sizeof(int16_t) : 0; j < audioBufferSize * 2; i += sizeof(int16_t), j += (sizeof(int16_t) * INPUT_CHANNEL_COUNTS)) //channel counts
                {
                    memcpy(&queueBuff[i], micBuff + j, sizeof(int16_t));
                }

                if (pThis->interruptQueue == true)
                {
                    nullBuffCount++;
                    if (nullBuffCount >= 8)
                    {
                        (void)piLogger->Output(&_hBasexErrorxDLF, "AUDIO::IN::FLOW: METHOD: THREAD - EPD callback from Server :: END OF SPEECH");

                        free(queueBuff);
                        queueBuff = nullptr;
                    }
                }

                gAudioInBlockingQueue.push(queueBuff);

                if (gAudioInBlockingQueue.getSize() >= 100)
                {
                    pThis->setCapturingSpeechAudio(false);
                    break;
                }

                memset(micBuff, 0, audioBufferSize * INPUT_CHANNEL_COUNTS);
                currentFrame = 0;
            }
        }

        free(micBuff);
        micBuff = nullptr;

        free(rBuff);
        rBuff = nullptr;
    }
}
