
/*
====================== HEADER ======================

class libwebsocketsListener
{
public:
    static libwebsocketsListener* getInstance() {
        if (!m_pInstance) {
            m_pInstance = new libwebsocketsListener;
        }
        return m_pInstance;
    }

    static void destory() {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

    static int ws_service_callback(lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len);

private:
    libwebsocketsListener() = default;
    ~libwebsocketsListener() = default;

    static libwebsocketsListener* m_pInstance;

};

====================== HEADER ======================
*/


int libwebsocketsListener::ws_service_callback(lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len)
{
    mzONLINExHANDLE *pOnlineHandle = OnlineHandle::getInstance()->get();

    if (reason < LWS_CALLBACK_GET_THREAD_ID || reason > LWS_CALLBACK_UNLOCK_POLL)
    {
        BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "LIB_WEBSOCKETS CALLBACK: %d", reason);

        if (pOnlineHandle->xWorkplace.rDestroy == true)
        {
            BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "already destroyed connection, return 0 ");

            return 0;
        }
    }

    switch (reason) {
    case LWS_CALLBACK_USER:
    {
        // Custom Callback Parameter. Obtaining "TID"
        BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, LOGGER_DEF_ARG_STRING_STRING, "LWS_CALLBACK_USER", (char *)in);

        break;
    }
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
        pOnlineHandle->xWorkplace.rCallback = 0;
        lws_callback_on_writable(pOnlineHandle->xSocket.pxWsi);

        break;
    }
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
        BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, " CONNECT ERROR (%s)", (char *)in);
    }
    case LWS_CALLBACK_CLOSED:
    {
      	pOnlineHandle->xWorkplace.rConnect = false;

        break;
    }
	// Server data received
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        if (len <= 0)
        {
            break;
        }

        nlohmann::json root;
      	
      	try {
            root = json::parse((char *) in);
        }
        catch (nlohmann::detail::exception &e) {
            BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, " Error [%s]", e.what());
            BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "Do not call TIMA callback function");
          
            break;
        }

        std::stringstream ssBodyData((char *) in);

        ssBodyData.seekg(0, std::ios::end);

        int size = ssBodyData.tellg();

        OnlineSignal.push(new WsRequestFinished(200, "OK", ccosOnlineBodyType::JSON, ssBodyData, size));

        pOnlineHandle->xWorkplace.rCallback = 1;
      
        (void) usleep(50 * 1000);

        break;
    }
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
        if (pOnlineHandle->xWorkplace.rDestroy == true)
        {
            break;
        }

        BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "LWS_CALLBACK_CLIENT_WRITEABLE");

        OnlineWebsockets *pOnlineWebsockets = OnlineWebsockets::getInstance();

        pOnlineHandle->xWorkplace.rConnect = true;

      	// Creating thread for sending audio packets
        if (pOnlineWebsockets->createThread((void *)pOnlineHandle) < 0)
        {
            BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "ERROR: CREATING BUFFER THREAD");

            exit(0);
        }

        break;
    }
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
    {
        if (pOnlineHandle->xWorkplace.rDestroy == true)
        {
            break;
        }

        unsigned char **p = (unsigned char **)in, *end = (*p) + len;

        pOnlineHandle->xConfig.axConfig.erase("URI:");
        pOnlineHandle->xConfig.axConfig.erase("Host:");
        pOnlineHandle->xConfig.axConfig.erase("MessageID:");

        for (auto&& iter: pOnlineHandle->xConfig.axConfig)
        {
            if (iter.first.empty() | iter.second.empty() == false)
            {
                BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "HANDSHAKE_HEADER [%s %s]", iter.first.data(), iter.second.data());

                if (lws_add_http_header_by_name(wsi, (const unsigned char *)iter.first.data(), (const unsigned char *)iter.second.data(), strlen(iter.second.data()), p, end))
                    return -1;
            }
        }

        break;
    }
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
    {

        int		nCode = 0, nValue = 0;
        char	*pReject = NULL, *ptEnd = NULL;
        int 	rValue = 0;

        if (len > 2)
        {
            nCode = ((unsigned char *)in)[0] * 0x100 + ((unsigned char *)in)[1];
            BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "SOCKET CLOSED - REASON(%d) [ %s ]", nCode, (char *)in+2);
        }


        switch (nCode) {
        case MZ_ONLINE_CTRL_DISCONNECTION_OK:
        case MZ_ONLINE_CTRL_DISCONNECTION_GOING_OK:
        {
            nValue = MZ_ONLINE_CTRL_RESULT_RESULT;
            break;
        }
        case MZ_ONLINE_CTRL_DISCONNECTION_REQ_ENTITY_LARGE:
        case MZ_ONLINE_CTRL_DISCONNECTION_UNSUPPORTED_VALUE:
        case MZ_ONLINE_CTRL_DISCONNECTION_REQ_TIMEOUT:
        case MZ_ONLINE_CTRL_DISCONNECTION_GATE_TIMEOUT:
        case MZ_ONLINE_CTRL_DISCONNECTION_CP_TIMEOUT_VALUE:
        {
            nValue = MZ_ONLINE_CTRL_RESULT_REJECTION;
            break;
        }
        case MZ_ONLINE_CTRL_DISCONNECTION_INTERNAL_SERVER:
        case MZ_ONLINE_CTRL_DISCONNECTION_BAD_GATEWAY:
        {
            nValue = MZ_ONLINE_CTRL_RESULT_TIMEOUT;
            break;
        }
        default:
            break;
        }

        pOnlineHandle->xCallback.cbFunction(pOnlineHandle->xCallback.pvInstance, MZ_ONLINE_CTRL_CALLBACK_TYPE_CLOSE, MZ_ONLINE_CTRL_CALLBACK_EVENT_TYPE, &nValue, NULL);

        pOnlineHandle->xWorkplace.rReqResCon = false;
        pOnlineHandle->xWorkplace.rConnect = false;
        pOnlineHandle->xWorkplace.rDestroy = true;

        BaseLogWrapper::getService()->Log(LOGGER_DEF_ONLINE, __FUNCTION__, "SOCKET CLOSED - disconect");

        break;
    }
    default:
        break;
    }

    return 0;
}
