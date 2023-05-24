char *CAudioRingBuffer::getMemoryBuffer(int* bufferSize, int channel)
{
    int requestBufferSize = *bufferSize * INPUT_CHANNEL_COUNTS;

    *bufferSize = 0;

    if (requestBufferSize > (getBufferConut() * mBufferSize))
    {
        return nullptr;
    }

    char *rawBuff = new char[requestBufferSize];
    char *retBuff = new char[requestBufferSize/INPUT_CHANNEL_COUNTS]; //channels

    int requiredBufferSize  = requestBufferSize;

    unsigned int indexGap   = (last - first);
    unsigned int idx        = (first * mBufferSize);

    if (indexGap > 0)
    {
        int storedBufferSize    = (indexGap * mBufferSize);
        int mockupBufferSize    = ((requiredBufferSize > storedBufferSize)? (requiredBufferSize - storedBufferSize) : 0);

        if (mockupBufferSize > 0)
        {
            memset(rawBuff, 0, mockupBufferSize);
            requiredBufferSize -= mockupBufferSize;
        }
    }
    else
    {
        int postBufferSize      = (last * mBufferSize);
        int preBufferSize       = (requiredBufferSize - postBufferSize);

        if (preBufferSize > 0)
        {
            int storedBufferSize    = ((this->getBufferConut() - first) * mBufferSize);
            int mockupBufferSize    = ((preBufferSize > storedBufferSize)? (preBufferSize - storedBufferSize) : 0);

            if (mockupBufferSize > 0)
            {
                memset(rawBuff, 0, mockupBufferSize);
                preBufferSize -= mockupBufferSize;
            }

            idx = (this->getBufferConut()*mBufferSize - preBufferSize);

            memcpy(&rawBuff[mockupBufferSize], &mBufferPtr[idx], preBufferSize);
            requiredBufferSize -= preBufferSize;
        }

        idx = ((last * mBufferSize) - requiredBufferSize);
    }

    memcpy(&rawBuff[requestBufferSize - requiredBufferSize], &mBufferPtr[idx], requiredBufferSize);

    for (int i = 0, j = (channel == 2)? sizeof(int16_t) : 0; j < requestBufferSize; i += sizeof(int16_t), j += (sizeof(int16_t) * INPUT_CHANNEL_COUNTS)) //channel counts
    {
        memcpy(&retBuff[i], rawBuff + j, sizeof(int16_t));
        *bufferSize += sizeof(int16_t);
    }

    delete[] rawBuff;
    rawBuff = nullptr;

    return retBuff;
}
