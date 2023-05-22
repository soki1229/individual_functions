/*
===================== HEADER =====================

define SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_GIV                       0x01
#define SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_GIV_SUR                  0x02
#define SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_SUR                      0x04
#define SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_SUR_GIVEN                0x08

===================== HEADER =====================
*/

MZINT DialogDataProduction::spreadContact(MZINT spreadType)
{
    SDS_Handle *                          pSDSHandle   = SDS_Handle::getInstance();
    list<pair<string, string>>::iterator  it;
    list<T_SDS_DP_VARIABLE *>::iterator   iter;

    BaseLogServices::getServices()->Log("INFO","DialogDataProduction","spreadContact()");

    for (auto&& it: pSDSHandle->dpList)
    {
        std::string givName = it.first;
        std::string famName = it.second;

        bool tGiv       = spreadType & SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_GIV;
        bool tGivSur    = spreadType & SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_GIV_SUR;
        bool tSur       = spreadType & SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_SUR;
        bool tSurGiv    = spreadType & SDS_DIALOG_DATA_PRODUCTION_SPREAD_TYPE_SUR_GIVEN;

        if (givName.empty()|famName.empty())
            tGivSur = false, tSurGiv = false;
        if (givName.empty())
            tGiv = false;
        if (famName.empty())
            tSur = false;
        if (!(tGiv|tSur))
            continue;

        if(this->rinterrupt == true)
            break;

        if (tGiv == true)
        {
            T_SDS_DP_VARIABLE * dpVariable = new T_SDS_DP_VARIABLE;

            dpVariable->rSlotID = SDS_SLOT_ID_LEX_VAD_G;
            dpVariable->nLexeme = tSur? SDS_CONTEXT_TYPE_PARTIAL_G : SDS_CONTEXT_TYPE_SINGLE;
            dpVariable->ptItem  = givName;

            mSpreadList.push_back(dpVariable);
        }

        if(this->rinterrupt == true)
            break;

        if (tGivSur == true)
        {
            T_SDS_DP_VARIABLE * dpVariable = new T_SDS_DP_VARIABLE;

            dpVariable->rSlotID = SDS_SLOT_ID_LEX_VAD_GF;
            dpVariable->nLexeme = SDS_CONTEXT_TYPE_FULL_GF;
            dpVariable->ptItem  = givName + '_' + famName;

            mSpreadList.push_back(dpVariable);
        }

        if (this->rinterrupt == true)
            break;

        if (tSur == true)
        {
            T_SDS_DP_VARIABLE * dpVariable = new T_SDS_DP_VARIABLE;

            dpVariable->rSlotID = SDS_SLOT_ID_LEX_VAD_F;
            dpVariable->nLexeme = givName.empty()? SDS_CONTEXT_TYPE_SINGLE : SDS_CONTEXT_TYPE_PARTIAL_F;
            dpVariable->ptItem  = famName;

            mSpreadList.push_back(dpVariable);
        }

        if (this->rinterrupt == true)
            break;

        if (tSurGiv == true)
        {
            T_SDS_DP_VARIABLE * dpVariable = new T_SDS_DP_VARIABLE;

            dpVariable->rSlotID = SDS_SLOT_ID_LEX_VAD_FG;
            dpVariable->nLexeme = SDS_CONTEXT_TYPE_FULL_FG;
            dpVariable->ptItem  = famName + '_' + givName;

            mSpreadList.push_back(dpVariable);
        }

        if (this->rinterrupt == true)
            break;

    }

    if (this->rinterrupt == true)
        return SDS_BASE_ERROR_NO_ERROR;

    /** Sort Name */
    BaseLogServices::getServices()->Log("INFO","DialogDataProduction","spreadContact() - SPREAD CONTACT COUNT : %d", mSpreadList.size());

    mSpreadList.sort([](const T_SDS_DP_VARIABLE * a, const T_SDS_DP_VARIABLE * b){ return (a->ptItem < b->ptItem); });

    BaseLogServices::getServices()->Log("INFO","DialogDataProduction","spreadContact() - CONTACT SORT DONE");

    if(this->rinterrupt == true)
        return SDS_BASE_ERROR_NO_ERROR;

    /** Repetition removal according to nameType */
    iter = mSpreadList.begin();
    iter++;

    while ((iter != mSpreadList.end()) && (this->rinterrupt != true))
    {
        T_SDS_DP_VARIABLE *dpNode;
        T_SDS_DP_VARIABLE *dpPrevNode;

        dpNode = *iter;
        iter--;
        dpPrevNode = *iter;
        iter++;

        if ((dpNode->ptItem.empty() == true) || (dpPrevNode->ptItem.empty() == true))
        {
            BaseLogServices::getServices()->Log("ERROR","DialogDataProduction","spreadContact() - data generate Error");
            return SDS_BASE_ERROR_DATA_ERROR;
        }

        if (dpNode->ptItem.compare(dpPrevNode->ptItem) == 0)
        {
            if (dpNode->nLexeme == dpPrevNode->nLexeme)
            {
                delete(*iter);
                iter = mSpreadList.erase(iter);
                iter--;
            }
            else if (dpNode->nLexeme <= dpPrevNode->nLexeme)
            {
                delete(*iter);
                iter = mSpreadList.erase(iter);
                iter--;
            }
            else
            {
                iter--;
                delete(*iter);
                iter = mSpreadList.erase(iter);
            }
        }

        ++iter;
    }

    return SDS_BASE_ERROR_NO_ERROR;
}
