#include <jni.h>
//#include stdio.h
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "linkkit_export.h"
#include "iot_import.h"

/** USER NOTIFICATION
 *  this sample code is only used for evaluation or test of the iLop project.
 *  Users should modify this sample code freely according to the product/device TSL, like
 *  property/event/service identifiers, and the item value type(type, length, etc...).
 *  Create user's own execution logic for specific products.
 */

#include "stdio.h"
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <linkkit_export.h>
#include <imports/iot_import_product.h>

#include <android/log.h>
#include <exports/iot_export_file_uploader.h>
#include <strings.h>

#define   LOG_TAG    "LOG_TEST"
#define   LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define   LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define   LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#endif

#include "linkkit_export.h"

#include "iot_import.h"

#ifndef bool
#define bool char
#endif

#define LINKKIT_PRINTF(...)  \
    do {                                                     \
        printf("\e[0;32m%s@line%d:\t", __FUNCTION__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\e[0m");                                   \
    } while (0)


/* specify ota buffer size for ota service, ota service will use this buffer for bin download. */
#define OTA_BUFFER_SIZE                  (512+1)

//人脸库属性全局变量
char  g_ProductKey[64] = {0};
char  g_DeviceName[64] = {0};
char  g_DeviceSecret[64] = {0};
char  g_ServerURL[256] = {0};
int   g_ServerPort = -1;
char  g_PackageName[128] = {0};
char  g_Me[128] = {0};
char  g_ClientId[128] = {0};
char  g_Token[128] = {0};
char  g_DeviceEncrypt[1024] = {0};
float g_SyncRate = 1.0;
char  g_faceID[64] = {0};
float g_score = 0;
char  *g_FaceDetectedData = NULL;
char  *g_FaceMatchedData = NULL;
int   g_outlen = 1024*1024;
bool  g_IsDetectedUploading = JNI_FALSE;
bool  g_IsMatchedUploading = JNI_FALSE;
char  *g_AddedUserInfoBuff = NULL;
pthread_mutex_t g_AddedUserInfoBuffLock;



char gc_licenseData[4096] = {0};
linkkit_facedoor_property_t g_stFaceDoorProperty = {0};
sample_context_t g_sample_context;
JavaVM *g_jvm;
jobject g_obj;


void post_event_cb(const void* thing_id, int respons_id, int code, const char* response_message, void* ctx)
{
    LOGI("This is callback func!\n");
}

void post_property_cb(const void* thing_id, int respons_id, int code, const char* response_message, void* ctx)
{
    LOGI("thing@%p: response arrived:\nid:%d\tcode:%d\tmessage:%s\n", thing_id, respons_id, code, response_message == NULL ? "NULL" : response_message);
}

/**
 * @auther
 * @brief 设备人脸比对事件上报
 *
 * @param[in] sample_context_t *sample
 * @param[in] char *pcContextData-设备端授权信息
 *
 * @return void
 */
static void post_OnMatched(sample_context_t* sample, char *pcUserPicID, char *pcFaceMatchedPicStorID, float fSimilarity)
{
    int  ret = 0;
    int  i = 0;
    char *pFaceID = NULL;
    char *pUserInfo = NULL;
    char *pEnd = NULL;
    char event_output_identifier[64];
    char acFaceGroupIDTmp[32] = {0};
    char userInfo[255] = {0};

    fSimilarity *= 100;
    /* 找出pcUserPicID的属性信息 */
    pthread_mutex_lock(&g_AddedUserInfoBuffLock);

    LOGI("pcUserPicID is [%s]+++g_AddedUserInfoBuff is [%s]\n", pcUserPicID, g_AddedUserInfoBuff);

    pFaceID = strstr(g_AddedUserInfoBuff, pcUserPicID);
    if (NULL == pFaceID)
    {
        LOGE("Err FaceID [%s]\n", pcUserPicID);
        pthread_mutex_unlock(&g_AddedUserInfoBuffLock);
        return;
    }
    LOGI("pFaceID is [%s]\n", pFaceID);

    pUserInfo = strstr(pFaceID, "userInfo");
    if (NULL == pUserInfo)
    {
        LOGE("userInfo is NULL\n");
        pthread_mutex_unlock(&g_AddedUserInfoBuffLock);
        return;
    }
    LOGI("pUserInfo is [%s]\n", pUserInfo);

    pEnd = pUserInfo+11;

    LOGI("pEnd is [%s]\n", pEnd);


    while ((pEnd[i] != '\"') && (pEnd[i] != '\0'))
    {
        i++;
    }

    strncpy(userInfo, pEnd, i);
    LOGI("userInfo is [%s]\n", userInfo);
    pthread_mutex_unlock(&g_AddedUserInfoBuffLock);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", "OnMatched", "UserInfo");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      userInfo, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", "OnMatched", "UserPicID");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      pcUserPicID, NULL);
    LOGI("event_output_identifier:%s\n", event_output_identifier);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", "OnMatched", "FaceMatchedPicStorID");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      pcFaceMatchedPicStorID, NULL);
    LOGI("event_output_identifier:%s\n", event_output_identifier);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", "OnMatched", "Similarity");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      &fSimilarity, NULL);
    LOGI("fSimilarity==%f\n", fSimilarity);
    LOGI("event_output_identifier:%s\n", event_output_identifier);

    strncpy(acFaceGroupIDTmp, g_stFaceDoorProperty.acFaceGroupID, strlen(g_stFaceDoorProperty.acFaceGroupID) - 1);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", "OnMatched", "GroupID");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      acFaceGroupIDTmp, NULL);

    LOGI("g_stFaceDoorProperty.acFaceGroupID=%s\n", acFaceGroupIDTmp);
    ret = linkkit_trigger_event(sample->thing, "OnMatched", post_event_cb);
    if (ret > 0)
    {
        LOGI("post_OnMatched--->send success:%d\n", ret);
    }
    else
    {
        LOGE("post_OnMatched--->send err:%d\n", ret);
    }

    return;
}


/**
 * @auther
 * @brief 设备人脸检测事件上报
 *
 * @param[in] sample_context_t *sample
 * @param[in] char *pcContextData-设备端授权信息
 *
 * @return void
 */
static void post_OnDetected(sample_context_t* sample, char *pcFaceMatchedPicStorID)
{
    int  ret = 0;
    char event_output_identifier[64];
    char acFaceGroupIDTmp[32] = {0};

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "OnDetect", "StoreID");
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      pcFaceMatchedPicStorID, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "OnDetect", "GroupID");

    strncpy(acFaceGroupIDTmp, g_stFaceDoorProperty.acFaceGroupID, strlen(g_stFaceDoorProperty.acFaceGroupID) - 1);

    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      acFaceGroupIDTmp, NULL);

    ret = linkkit_trigger_event(sample->thing, "OnDetect", post_event_cb);
    if (ret > 0)
    {
        LOGI("post_OnDetected--->send id:%d\n", ret);
    }
    else
    {
        LOGE("post_OnDetected--->send err:%d\n", ret);
    }

    return;
}

/**
 * @auther
 * @brief 属性信息写配置文件
 *
 * @return 0-成功，-1-失败
 */
static int write_property(void)
{
    FILE *pf = NULL;
    char cBuff[128];
    int ret = 0;

    pf = fopen("/sdcard/property", "w+");
    if (NULL == pf)
    {
        LOGE("failed to open file property\n");
        return -1;
    }

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%d", "FaceSetSize", g_stFaceDoorProperty.iFaceSetSize);
    LOGI("Write cBuff ==%s\n", cBuff);
    fputs(cBuff, pf);
    fputs("\n",pf);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%d", "FaceSetPicStoreAbility", g_stFaceDoorProperty.iFaceSetPicStoreAbility);
    LOGI("Write cBuff ==%s\n", cBuff);
    fputs(cBuff, pf);
    fputs("\n",pf);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%lf", "FaceSetOnMatchThreshOld", g_stFaceDoorProperty.dOnMatchThreshOld);
    LOGI("Write cBuff ==%s\n", cBuff);
    fputs(cBuff, pf);
    fputs("\n",pf);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%s", "GroupID", g_stFaceDoorProperty.acFaceGroupID);
    LOGI("Write cBuff acFaceGroupID ==%s\n", cBuff);
    ret = fputs(cBuff, pf);
    LOGI("retLen = %d\n", ret);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%s", "FaceSetID", g_stFaceDoorProperty.acFaceSetID);
    LOGI("Write cBuff ==%s\n", cBuff);
    ret = fputs(cBuff, pf);
    LOGI("retLen = %d\n", ret);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%s", "FaceSetAlgorithmVersion", g_stFaceDoorProperty.acFaceSetAlgorithmVersion);
    LOGI("Write cBuff ==%s\n", cBuff);
    ret = fputs(cBuff, pf);
    LOGI("retLen = %d\n", ret);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%s", "FaceSetSign", g_stFaceDoorProperty.acFaceSetMD5Sign);
    LOGI("Write cBuff ==%s\n", cBuff);
    ret = fputs(cBuff, pf);
    LOGI("retLen = %d\n", ret);

    memset(cBuff, 0, sizeof(cBuff));
    snprintf(cBuff, sizeof(cBuff), "%s:%s", "AddedFacesStoreID", g_stFaceDoorProperty.acAddedFacesStoreID);
    LOGI("Write cBuff AddedFacesStoreID ==%s\n", cBuff);
    ret = fputs(cBuff, pf);
    LOGI("retLen = %d\n", ret);

    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "FaceSetSize", &g_stFaceDoorProperty.iFaceSetSize,NULL);
    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "FaceSetPicStoreAbility", &g_stFaceDoorProperty.iFaceSetPicStoreAbility,NULL);
    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "FaceSetAlgorithmVersion", g_stFaceDoorProperty.acFaceSetAlgorithmVersion,NULL);
    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "FaceSetID", g_stFaceDoorProperty.acFaceSetID,NULL);
    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "FaceSetMD5Sign", g_stFaceDoorProperty.acFaceSetMD5Sign,NULL);
    linkkit_set_value(linkkit_method_set_property_value, g_sample_context.thing, "OnMatchThreshOld", &g_stFaceDoorProperty.dOnMatchThreshOld,NULL);

    fclose(pf);
    return 0;
}

/**
 * 从文件中获取'属性'数据，写入到全局变量g_stFaceDoorProperty中
 */
static int read_property(void)
{
    FILE *pf = NULL;
    char *pcBegin = NULL;
    char *pcEnd   = NULL;
    char cBuff[128];
    char cBuffTmp[64];
    char cBuffTmp1[64];

    pf = fopen("/sdcard/property", "r");
    if (NULL == pf)
    {
        /* 设置属性文件默认参数 */
        memset(&g_stFaceDoorProperty, 0, sizeof(g_stFaceDoorProperty));
        g_stFaceDoorProperty.iFaceSetPicStoreAbility = 200000;
        g_stFaceDoorProperty.dOnMatchThreshOld = 85.00;
        strncpy(g_stFaceDoorProperty.acFaceSetID, "FaceDetect\n", strlen("FaceDetect\n"));
        strncpy(g_stFaceDoorProperty.acFaceSetAlgorithmVersion, "1.1.1\n", strlen("1.1.1\n"));
        strncpy(g_stFaceDoorProperty.acFaceSetMD5Sign, "AAAAAAAAAABBBBBBBBBBCCCCCCCCCC32\n", strlen("AAAAAAAAAABBBBBBBBBBCCCCCCCCCC32\n"));
        strncpy(g_stFaceDoorProperty.acFaceGroupID, "\n", strlen("\n"));
        write_property();
        pf = fopen("/sdcard/property", "r");
        if (NULL == pf)
        {
            LOGE("read_property---->Failed to open file property! Return from read_property!\n");
            return -1;
        }
    }

    memset(cBuff, 0, sizeof(cBuff));
    memset(cBuffTmp, 0, sizeof(cBuffTmp));
    memset(cBuffTmp1, 0, sizeof(cBuffTmp1));

    while(fgets(cBuff, sizeof(cBuff), pf))
    {
        LOGI("cBuff info:%s\n",cBuff);

        pcBegin = &cBuff[0];
        pcEnd = strstr(cBuff,":")+1;
        if (NULL == pcEnd)
        {
            fclose(pf);
            LOGE("read_property---->pcEnd is NULL, return\n");
            return -1;
        }

        memcpy(cBuffTmp, cBuff, pcEnd-pcBegin);
        pcBegin = pcEnd;
        pcEnd = &cBuff[0]+strlen(cBuff);
        memcpy(cBuffTmp1, pcBegin, pcEnd-pcBegin);

        if (0 == strcmp("FaceSetSize:", cBuffTmp))
        {
            g_stFaceDoorProperty.iFaceSetSize = atoi(cBuffTmp1);
        }
        else if (0 == strcmp("FaceSetPicStoreAbility:", cBuffTmp))
        {
            g_stFaceDoorProperty.iFaceSetPicStoreAbility = atoi(cBuffTmp1);
        }
        else if (0 == strcmp("FaceSetID:", cBuffTmp))
        {
            strncpy(g_stFaceDoorProperty.acFaceSetID, cBuffTmp1, sizeof(g_stFaceDoorProperty.acFaceSetID));
        }
        else if (0 == strcmp("AddedFacesStoreID:", cBuffTmp))
        {
            strncpy(g_stFaceDoorProperty.acAddedFacesStoreID, cBuffTmp1, sizeof(g_stFaceDoorProperty.acAddedFacesStoreID));
        }
        else if (0 == strcmp("FaceSetAlgorithmVersion:", cBuffTmp))
        {
            strncpy(g_stFaceDoorProperty.acFaceSetAlgorithmVersion, cBuffTmp1, sizeof(g_stFaceDoorProperty.acFaceSetAlgorithmVersion));
        }
        else if (0 == strcmp("FaceSetSign:", cBuffTmp))
        {
            strncpy(g_stFaceDoorProperty.acFaceSetMD5Sign, cBuffTmp1, sizeof(g_stFaceDoorProperty.acFaceSetMD5Sign));
        }
        else if (0 == strcmp("OnMatchThreshOld:", cBuffTmp))
        {
            g_stFaceDoorProperty.dOnMatchThreshOld = atof(cBuffTmp1);
        }
        else if (0 == strcmp("GroupID:", cBuffTmp))
        {
            strncpy(g_stFaceDoorProperty.acFaceGroupID, cBuffTmp1, sizeof(g_stFaceDoorProperty.acFaceGroupID));
        }

        printf("\n\n");
        LOGI("iFaceSetSize-%d\n",g_stFaceDoorProperty.iFaceSetSize);
        LOGI("iFaceSetPicStoreAbility-%d\n",g_stFaceDoorProperty.iFaceSetPicStoreAbility);
        LOGI("dOnMatchThreshOld-%f\n",g_stFaceDoorProperty.dOnMatchThreshOld);
        LOGI("acFaceSetAlgorithmVersion-%s\n",g_stFaceDoorProperty.acFaceSetAlgorithmVersion);
        LOGI("acFaceSetID-%s\n",g_stFaceDoorProperty.acFaceSetID);
        LOGI("acFaceSetMD5Sign-%s\n",g_stFaceDoorProperty.acFaceSetMD5Sign);
        LOGI("acAddedFacesStoreID-%s\n",g_stFaceDoorProperty.acAddedFacesStoreID);
        LOGI("acFaceGroupID-%s\n",g_stFaceDoorProperty.acFaceGroupID);

        memset(cBuff, 0, sizeof(cBuff));
        memset(cBuffTmp, 0, sizeof(cBuffTmp));
        memset(cBuffTmp1, 0, sizeof(cBuffTmp1));
    }

    fclose(pf);

    return 0;
}

/**
 * @auther
 * @brief 解析TMP下发的服务信息
 *
 * @param[in] sample_context_t *sample
 * @param[in] void* thing
 * @param[in] char* service_identifier[服务标识符]
 * @param[in] int request_id
 *
 * @return 0-成功，-1-失败
 */
static int handle_service_input(sample_context_t* sample, const void* thing, const char* service_identifier,
                                int request_id, int rrpc)
{
    char identifier[128]    = {0};
    bool doAuthOK = 1;
    int  status = 0;

    LOGI("This is func handle_service_input rrpc===%d  service_identifier=%s\n", rrpc, service_identifier);

    /* 向人脸库中添加一张人脸图片 */
    if (strcmp(service_identifier, "AddOneFacePic") == 0)
    {
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "UserPicID");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->cUserID, NULL);

        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "FaceImageURL");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->FaceImageURL, NULL);

        LOGI("Add sample->cUserID is [%s]\n", sample->cUserID);
        LOGI("Add sample->FaceImageURL is [%s]\n", sample->FaceImageURL);

        //todo: 向人脸库中添加人脸图片流程
        JNIEnv *env = NULL;
        bool isAttached = JNI_FALSE;

        status =(*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_4);
        if(status < 0)
        {
            (*g_jvm)->AttachCurrentThread(g_jvm,&env, NULL);//将当前线程注册到虚拟机中．
            isAttached = JNI_TRUE;
        }

        jclass claxx = (*env)->FindClass(env, "pc/tingyi/facedoor/FaceDoor");
        jmethodID method= (*env)->GetMethodID(env, claxx, "OnFacePicUrl", "(Ljava/lang/String;)V");
        jstring FacePictureURL = (*env)->NewStringUTF(env, sample->FaceImageURL);
        (*env)->CallVoidMethod(env, g_obj, method, FacePictureURL);

        g_stFaceDoorProperty.iFaceSetSize += 1;

        write_property();
    }
    /* 向人脸库中删除一张人脸图片 */
    else if(strcmp(service_identifier, "RemoveOneFacePic") == 0)
    {
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "UserPicID");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->cUserID, NULL);
        LOGI("Del sample->cUserID is [%s]\n", sample->cUserID);
        //todo: 向人脸库中添加人脸图片流程

        g_stFaceDoorProperty.iFaceSetSize -= 1;

        write_property();
    }

    /* 删除人脸库中所有的图片 */
    else if(strcmp(service_identifier, "DeleteAllFacePic") == 0)
    {
        g_stFaceDoorProperty.iFaceSetSize = 0;
        //todo: 删除人脸库中所有的图片

        write_property();
    }

    /* 设备VerifySDK授权文件下发 */
    else if (strcmp(service_identifier, "AuthVerifySDK") == 0)
    {
        struct timeval start, end;

        gettimeofday( &start, NULL );
        LOGI("Beginning time--->%d-%d", start.tv_sec, start.tv_usec);

        char    service_output_identifier[64] = {0};
        int     StatusCode                    = 0;
        char    StatusCodeDescription[256]    = {0};
        snprintf(service_output_identifier, sizeof(service_output_identifier), "%s.%s", "AuthVerifySDK", "DoAuthorized");

        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "LicenseData");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->LisenceData, NULL);
        LOGI("sample->LisenceData is [%s]\n", sample->LisenceData);

        snprintf(gc_licenseData, sizeof(gc_licenseData), "%s", sample->LisenceData);
        LOGI("gc_licenseData is [%s]\n", gc_licenseData);

        /* StatusCode一般不能修改，和物模型中定义的保持一致 */
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "StatusCode");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &StatusCode, NULL);
        LOGI("sample->StatusCode is [%d]\n", StatusCode);

        /* StatusCodeDescription一般不能修改，和物模型中定义的保持一致 */
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "StatusCodeDescription");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, StatusCodeDescription, NULL);
        LOGI("sample->StatusCodeDescription is [%s]\n", StatusCodeDescription);

        /* 状态码为200表示授权成功，只有为200时，licenseData为非空 */
        if (200 == StatusCode)
        {
            JNIEnv *env = NULL;

            bool isAttached = JNI_FALSE;

            status = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_4);
            if (status < 0)
            {
                //将当前线程注册到虚拟机中
                (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
                isAttached = JNI_TRUE;
            }

            jclass claxx = (*env)->FindClass(env, "pc/tingyi/facedoor/FaceDoor");
            jmethodID method = (*env)->GetMethodID(env, claxx, "OnMessage", "(ILjava/lang/String;Ljava/lang/String;)V");
            if (NULL == method)
            {
                LOGI("ERR: MethodID is NULL!\n");
                return 0;
            }
            jstring licenseData = (*env)->NewStringUTF(env, gc_licenseData);
            jstring statusCodeDesc = (*env)->NewStringUTF(env, StatusCodeDescription);
            (*env)->CallVoidMethod(env, g_obj, method, StatusCode, licenseData, statusCodeDesc);
        }
        /* 向平台回复响应消息 */
        LOGI("service_output_identifier@@@ is [%s]\n", service_output_identifier);
        if (0 != linkkit_set_value(linkkit_method_set_service_output_value, thing,
                                   service_output_identifier,
                                   &doAuthOK, NULL))
        {
            LOGE("service_output_identifier@@@ err is [%s]\n", service_output_identifier);
        }
        linkkit_answer_service(thing, "AuthVerifySDK", request_id, 200, rrpc);

        gettimeofday( &end, NULL );
        LOGI("Ending time %d-%d", end.tv_sec, end.tv_usec);

        ///* 向平台回复响应消息 */
        linkkit_answer_service(sample->thing, "AuthVerifySDK", request_id, doAuthOK, rrpc);
    }
    /* 同步人脸图片：批量布控人脸图 */
    else if (strcmp(service_identifier, "SyncFacePictures") == 0)
    {
        pthread_t pid;
        char service_output_identifier[64] = {0};
        int  SyncStatus = 0;

        /* 布控进度不为100%时，不接受布控 */
        if (g_SyncRate != 1.00)
        {
            SyncStatus = 1;   //状态：布控中
        }

        snprintf(service_output_identifier, sizeof(service_output_identifier), "%s.%s",
                                                   "SyncFacePictures", "SyncPicStatus");

        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "FacePicURL");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->FacePicturesURL, NULL);
        LOGI("sample->FacePicturesURL is [%s]\n", sample->FacePicturesURL);
        linkkit_set_value(linkkit_method_set_service_output_value, thing, service_output_identifier,
                          &SyncStatus, NULL);
        /* 向平台回复响应消息 */
        linkkit_answer_service(thing, "SyncFacePictures", request_id, 200, rrpc);

        if (SyncStatus == 0)
        {
            //todo: 回调java方法，下载批量布控人脸图文件
            JNIEnv *env = NULL;
            bool isAttached = JNI_FALSE;

            status =(*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_4);
            if(status < 0)
            {
                (*g_jvm)->AttachCurrentThread(g_jvm,&env, NULL);//将当前线程注册到虚拟机中．
                isAttached = JNI_TRUE;
            }

            jclass claxx = (*env)->FindClass(env, "pc/tingyi/facedoor/FaceDoor");
            jmethodID method= (*env)->GetMethodID(env, claxx, "OnSyncFacePictures", "(Ljava/lang/String;)V");
            jstring FacePicturesURL = (*env)->NewStringUTF(env, sample->FacePicturesURL);

            (*env)->CallVoidMethod(env, g_obj, method, FacePicturesURL);
            //write_property();
        }
    }
    /* 查询布控成功的人脸信息 */
    else if (strcmp(service_identifier, "QueryAddedUserInfo") == 0)
    {
        int  SyncStatus = 0;  //状态：非布控状态
        char service_output_identifier[64] = {0};
        /* 布控进度不为100%时，直接返回状态；如果不是在布控中则需要重新将AddUserInfo上传OSS重新获取新的StoreID传给平台*/
        if (g_SyncRate != 1.00)
        {
            LOGI("g_SyncRate=%f\n", g_SyncRate);
            SyncStatus = 1;   //状态：布控中
        }
        snprintf(service_output_identifier, sizeof(service_output_identifier), "%s.%s", "QueryAddedUserInfo", "StoreID");
        LOGI("service_output_identifier===%s\n", service_output_identifier);
        LOGI("g_AddedFaceStorID###%s\n", g_stFaceDoorProperty.acAddedFacesStoreID);

        linkkit_set_value(linkkit_method_set_service_output_value, thing, service_output_identifier,
                          g_stFaceDoorProperty.acAddedFacesStoreID, NULL);

        memset(service_output_identifier, 0, 64);
        snprintf(service_output_identifier, sizeof(service_output_identifier), "%s.%s", "QueryAddedUserInfo", "SyncPicStatus");
        linkkit_set_value(linkkit_method_set_service_output_value, thing, service_output_identifier,
                          &SyncStatus, NULL);
        /* 向平台回复响应消息 */
        linkkit_answer_service(thing, "QueryAddedUserInfo", request_id, 200, rrpc);

        write_property();
    }
    /* 查询布控进度百分比 */
    else if (strcmp(service_identifier, "QuerySyncPicSchedule") == 0)
    {
        LOGI("Call QuerySyncPicSchedule g_SyncRate = %f\n", g_SyncRate);
        char service_output_identifier[64] = {0};
        double SyncRat = 100*((double)g_SyncRate);
        snprintf(service_output_identifier, sizeof(service_output_identifier), "%s.%s",
                 "QuerySyncPicSchedule", "Rate");
        linkkit_set_value(linkkit_method_set_service_output_value, thing, service_output_identifier,
                          &SyncRat, NULL);

        /* 向平台回复响应消息 */
        linkkit_answer_service(thing, "QuerySyncPicSchedule", request_id, 200, rrpc);
    }
    /* 云端比对结果 */
    else if (strcmp(service_identifier, "FaceCompareResult") == 0)
    {
        char faceID[64] = {0};
        float similarity = 0.0;

        memset(identifier, 0, sizeof(identifier));
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "Similarity");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &similarity, NULL);

        memset(identifier, 0, sizeof(identifier));
        snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "FaceID");
        linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, faceID, NULL);

        LOGI("similarity:%f\n", similarity);
        LOGI("faceID:%s\n", faceID);
        struct timeval start;

        gettimeofday(&start, NULL);
        LOGI("FaceCompareResult time %d-%d", start.tv_sec, start.tv_usec);

        /* 向平台回复响应消息 */
        linkkit_answer_service(thing, "FaceCompareResult", request_id, 200, rrpc);
    }

    return 0;
}

#ifdef SERVICE_OTA_ENABLED
/* callback function for fota service. */
static void fota_callback(service_fota_callback_type_t callback_type, const char* version)
{
    sample_context_t* sample_ctx;

    assert(callback_type < service_fota_callback_type_number);

    sample_ctx = &g_sample_context;

    /* temporarily disable thing when ota service invoked */
    sample_ctx->thing_enabled = 0;

    linkkit_invoke_ota_service(sample_ctx->ota_buffer, OTA_BUFFER_SIZE);

    sample_ctx->thing_enabled = 1;

    /* reboot the device... */
}
#endif /* SERVICE_OTA_ENABLED */
#ifdef LOCAL_CONN_ENABLE
static int on_connect(void* ctx, int cloud)
#else
static int on_connect(void* ctx)
#endif
{
    sample_context_t* sample_ctx = ctx;

#ifdef LOCAL_CONN_ENABLE
    if (cloud) {
        sample_ctx->cloud_connected = 1;
    } else {
        sample_ctx->local_connected = 1;
    }
    LOGI("%s is connected\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 1;
    LOGI("%s is connected\n", "cloud");
#endif

    return 0;
}

#ifdef LOCAL_CONN_ENABLE
static int on_disconnect(void* ctx, int cloud)
#else
static int on_disconnect(void* ctx)
#endif
{
    sample_context_t* sample_ctx = ctx;

#ifdef LOCAL_CONN_ENABLE
    if (cloud) {
        sample_ctx->cloud_connected = 0;
    } else {
        sample_ctx->local_connected = 0;
    }
    LOGI("%s is disconnect\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 0;
    LOGI("%s is disconnect\n", "cloud");
#endif
    return 0;
}

static int raw_data_arrived(const void* thing_id, const void* data, int len, void* ctx)
{
    char raw_data[128] = {0};

    LOGI("raw data arrived,len:%d\n", len);

    /* do user's raw data process logical here. */

    /* ............................... */

    /* user's raw data process logical complete */

    snprintf(raw_data, sizeof(raw_data), "test down raw reply data %lld", (long long)HAL_UptimeMs());

    linkkit_invoke_raw_service(thing_id, 0, raw_data, strlen(raw_data));

    return 0;
}

static int thing_create(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    LOGI("new thing@%p created.\n", thing_id);
    sample_ctx->thing = thing_id;

    return 0;
}

static int thing_enable(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->thing_enabled = 1;

    return 0;
}

static int thing_disable(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->thing_enabled = 0;

    return 0;
}

#ifdef RRPC_ENABLED
static int thing_call_service(const void* thing_id, const char* service, int request_id, int rrpc, void* ctx)
#else
static int thing_call_service(const void* thing_id, const char* service, int request_id, void* ctx)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = ctx;

    LOGI("Call thing_call_service\n");
    handle_service_input(sample_ctx, thing_id, service, request_id, rrpc);

    return 0;
}

static int thing_prop_changed(const void* thing_id, const char* property, void* ctx)
{
    char* value_str = NULL;
    char property_buf[64] = {0};

    /* get new property value */
    if (strstr(property, "HSVColor") != 0)
    {
        int hue, saturation, value;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Hue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &hue, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Saturation");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &saturation, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Value");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &value, &value_str);

        LOGI("property(%s), Hue:%d, Saturation:%d, Value:%d\n", property, hue, saturation, value);

        /* XXX: do user's process logical here. */
    }
    else if (strstr(property, "HSLColor") != 0)
    {
        int hue, saturation, lightness;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Hue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &hue, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Saturation");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &saturation, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Lightness");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &lightness, &value_str);

        LOGI("property(%s), Hue:%d, Saturation:%d, Lightness:%d\n", property, hue, saturation, lightness);
        /* XXX: do user's process logical here. */
    }
    else if (strstr(property, "RGBColor") != 0)
    {
        int red, green, blue;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Red");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &red, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Green");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &green, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Blue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &blue, &value_str);

        LOGI("property(%s), Red:%d, Green:%d, Blue:%d\n", property, red, green, blue);
        /* XXX: do user's process logical here. */
    }
    else
    {
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property, NULL, &value_str);

        LOGI("property(%s) new value set: %s\n", property, value_str);
    }

    /* do user's process logical here. */
    linkkit_post_property(thing_id, property, post_property_cb);
    return 0;
}

static int linkit_data_arrived(const void* thing_id, const void* params, int len, void* ctx)
{
    LOGI("thing@%p: masterdev_linkkit_data(%d byte): %s\n", thing_id, len, (const char*)params);
    return 0;
}

static linkkit_ops_t alink_ops = {
        .on_connect           = on_connect,
        .on_disconnect        = on_disconnect,
        .raw_data_arrived     = raw_data_arrived,
        .thing_create         = thing_create,
        .thing_enable         = thing_enable,
        .thing_disable        = thing_disable,
        .thing_call_service   = thing_call_service,
        .thing_prop_changed   = thing_prop_changed,
        .linkit_data_arrived  = linkit_data_arrived,
};

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0) {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}


static int post_all_prop(sample_context_t* sample)
{
    return linkkit_post_property(sample->thing, NULL, post_property_cb);
}

static int is_active(sample_context_t* sample_ctx)
{
#ifdef LOCAL_CONN_ENABLE
    return (sample_ctx->cloud_connected && sample_ctx->thing_enabled) || (sample_ctx->local_connected && sample_ctx->thing_enabled);
#else
    return sample_ctx->cloud_connected && sample_ctx->thing_enabled;
#endif
}


static void NeedAuth(char * PackageName, char * ClientID, char * PublicKey)
{
    int  ret = 0;
    char event_output_identifier[128];

    LOGI("*PackageName===%s\n", PackageName);
    LOGI("*ClientID===%s\n", ClientID);
    LOGI("*PublicKey===%s\n", PublicKey);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "NeedAuthVerifySDK", "PackageName");

    LOGI("*event_output_identifier---->%s\n", event_output_identifier);

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      PackageName, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "NeedAuthVerifySDK", "ClientID");

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      ClientID, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "NeedAuthVerifySDK", "PublicKey");

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      PublicKey, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
                 "NeedAuthVerifySDK", "Token");

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      g_Token, NULL);

    ret = linkkit_trigger_event(g_sample_context.thing, "NeedAuthVerifySDK", NULL);
    if (ret > 0)
    {
        LOGI("send success:%d\n", ret);
    }
    else
    {
        LOGE("send err id:%d\n", ret);
    }
}

/**
 *  设备授权事件上报(携带可逆设备加密信息)
 */
static void NeedAuthV2()
{
    int  ret = 0;
    char event_output_identifier[128];

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "NeedAuthVerifySDK", "Token");

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      g_Token, NULL);

    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             "NeedAuthVerifySDK", "DeviceEncrypt");

    linkkit_set_value(linkkit_method_set_event_output_value,
                      g_sample_context.thing,
                      event_output_identifier,
                      g_DeviceEncrypt, NULL);

    ret = linkkit_trigger_event(g_sample_context.thing, "NeedAuthVerifySDK", NULL);
    if (ret > 0)
    {
        LOGI("send success:%d\n", ret);
    }
    else
    {
        LOGE("send err id:%d\n", ret);
    }
}

int file_upload_callback1(int result ,char *store_id, void *user_data)
{
    if (result == 0)
    {
        LOGI("file callback1 name:%s upload success: %s\n", (char *)user_data, store_id);
        memcpy(g_stFaceDoorProperty.acAddedFacesStoreID, store_id, strlen(store_id));
        LOGI("g_stFaceDoorProperty.acAddedFacesStoreID===%s\n", g_stFaceDoorProperty.acAddedFacesStoreID);
        write_property();
    }
    else
    {
        LOGE("file_upload_callback1:%s upload fail: %d\n", (char *)user_data, result);
    }

    return 1;
}

void* thread1(void *user_data)
{
    char *file_name = (char *)user_data;
    int ret = -1;
    int type = 0; /*0->file, 1-> log*/

    if (file_name != NULL)
    {
        LOGI("thread1 filename :%s\n", file_name);
        ret = iotx_upload_file_async(file_name, type, file_upload_callback1, user_data);
        LOGI("the ret of iotx_upload_file_async is %d\n", ret);
    }

    return NULL;
}

int onMatched_Face_upload_callback(int result ,char *store_id, void *user_data)
{
    if (result == 0)
    {
        LOGI("onMatched_Face_upload_callback:%s upload success: %s\n", (char *)user_data, store_id);

        post_OnMatched(&g_sample_context, g_faceID, store_id, g_score);
    }
    else
    {
        LOGI("onMatched_Face_upload_callback:%s upload fail: %d\n", (char *)user_data, result);
    }

    g_IsMatchedUploading = JNI_FALSE;

    return 1;
}

void* thread2(void *user_data)
{
    char *file_name = (char *)user_data;
    int ret = -1;
    int type = 0; /*0->file, 1-> log*/

    if (file_name != NULL)
    {
        LOGI("thread2 filename :%s\n", file_name);
        ret = iotx_upload_file_async(file_name, type, onMatched_Face_upload_callback, user_data);
        LOGI("the ret of iotx_upload_file_async is %d\n", ret);
    }

    return NULL;
}

int onMatched_Face_upload_callbackFD(int result ,char *store_id, void *user_data)
{
    if (result == 0)
    {
        LOGI("onMatched_Face_upload_callbackFD:%s upload success: %s\n", (char *)user_data,
             store_id);

        post_OnDetected(&g_sample_context, store_id);
    }
    else
    {
        LOGI("onMatched_Face_upload_callbackFD:%s upload fail: %d\n", (char *)user_data, result);
    }
    g_IsDetectedUploading = JNI_FALSE;

    return 1;
}

void* thread2FD(void *user_data)
{
    char *file_name = (char *)user_data;
    int ret = -1;
    int type = 0; /*0->file, 1-> log*/

    if (file_name != NULL)
    {
        LOGI("thread2FD filename :%s\n", file_name);

        ret = iotx_upload_file_async(file_name, type, onMatched_Face_upload_callbackFD, user_data);

        LOGI("the ret of iotx_upload_file_async is %d\n", ret);
    }

    return NULL;
}

void* thread_http2(void *user_data)
{
    LOGI("Call thread_http2");

    device_conn_info conn_info;
    memset(&conn_info, 0, sizeof(device_conn_info));
    conn_info.product_key = g_ProductKey;
    conn_info.device_name = g_DeviceName;
    conn_info.device_secret = g_DeviceSecret;
    conn_info.url = g_ServerURL;
    conn_info.port = g_ServerPort;

    LOGI("conn_info.product_key===[%s]\n", conn_info.product_key);
    LOGI("conn_info.device_name===[%s]\n", conn_info.device_name);
    LOGI("conn_info.device_secret===[%s]\n", conn_info.device_secret);
    LOGI("conn_info.url===[%s]\n", conn_info.url);
    LOGI("conn_info.port===[%d]\n", conn_info.port);

    iotx_http2_upload_file_init(&conn_info);
    return NULL;
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_setDeviceInfo(JNIEnv *env, jobject instance, jstring pdKey_,
                                               jstring dvKey_, jstring dvSec_, jstring productKey_,
                                               jstring token_,
                                               jstring deviceKey_, jstring deviceSecret_,
                                               jstring ServerURL_, jint ServerPort,
                                               jobject MessageCallbackTmp) {
    sample_context_t* sample_ctx = &g_sample_context;
    int execution_time = 0;
    int get_tsl_from_cloud = 1;
    int exit = 0;
    int bSend = 0;
    pthread_t  pid;
    unsigned long long now = 0;
    unsigned long long prev_sec = 0;
    fpos_t fpos; //当前位置
    int    fileLenth = 0;
    const char *pdKey = (*env)->GetStringUTFChars(env, pdKey_, 0);
    const char *dvKey = (*env)->GetStringUTFChars(env, dvKey_, 0);
    const char *dvSec = (*env)->GetStringUTFChars(env, dvSec_, 0);
    const char *productKey = (*env)->GetStringUTFChars(env, productKey_, 0);
    const char *token = (*env)->GetStringUTFChars(env, token_, 0);
    const char *deviceKey = (*env)->GetStringUTFChars(env, deviceKey_, 0);
    const char *deviceSecret = (*env)->GetStringUTFChars(env, deviceSecret_, 0);
    const char *ServerURL = (*env)->GetStringUTFChars(env, ServerURL_, 0);

    strncpy(g_ProductKey, pdKey, sizeof(g_ProductKey));
    strncpy(g_DeviceName, dvKey, sizeof(g_DeviceName));
    strncpy(g_DeviceSecret, dvSec, sizeof(g_DeviceSecret));
    strncpy(g_PackageName, productKey, sizeof(g_PackageName));
    strncpy(g_Me, deviceKey, sizeof(g_Me));
    strncpy(g_ClientId, deviceSecret, sizeof(g_ClientId));
    strncpy(g_ServerURL, ServerURL, sizeof(g_ServerURL));
    strncpy(g_Token, token, sizeof(g_Token));
    g_ServerPort = ServerPort;

    LOGI("Java_pc_tingyi_facedoor_FaceDoor_setDeviceInfo Token == %s\n", g_Token);

    do
    {
        pthread_mutex_init(&g_AddedUserInfoBuffLock, NULL);

        if (NULL == g_AddedUserInfoBuff)
        {
            FILE *p = fopen("/sdcard/AddedUser", "rb");

            if (p != NULL)
            {
                fgetpos(p, &fpos); //获取当前位置
                fseek(p, 0, SEEK_END);
                fileLenth = ftell(p);
                fsetpos(p,&fpos); //恢复之前的位置

                g_AddedUserInfoBuff = (char *)malloc(fileLenth+1);
                if (g_AddedUserInfoBuff == NULL)
                {
                    LOGE("No Memory!\n");
					fclose(p);
                    break;
                }
                bzero(g_AddedUserInfoBuff, fileLenth+1);

                fgets(g_AddedUserInfoBuff, fileLenth, p);
                fclose(p);
            }

        }

        if (NULL == g_FaceDetectedData)
        {
            g_FaceDetectedData = (char *) malloc(g_outlen);

            if (NULL == g_FaceDetectedData)
            {
                LOGE("ERR: Malloc mem failed!\n");
            }
        }

        if (NULL == g_FaceMatchedData)
        {
            g_FaceMatchedData = (char*)malloc(g_outlen);

            if (NULL == g_FaceMatchedData)
            {
                LOGE("ERR: Malloc mem failed!\n");
            }
        }



        //读取property属性文件
        read_property();

        (*env)->GetJavaVM(env, &g_jvm);
        g_obj = (*env)->NewGlobalRef(env, instance);

        //起HTTP2线程
        if (0 != pthread_create(&pid, NULL, thread_http2, NULL))
        {
            LOGE("thread_http2 create failed!\n");
            break;
        }
        execution_time = execution_time < 1 ? 1 : execution_time;
        LOGI("sample execution time: %d minutes\n", execution_time);
        LOGI("%s tsl from cloud\n", get_tsl_from_cloud == 0 ? "Not get" : "get");
        HAL_SetProductKey(pdKey);
        HAL_SetDeviceName(dvKey);
        HAL_SetDeviceSecret(dvSec);

        memset(sample_ctx, 0, sizeof(sample_context_t));
        sample_ctx->thing_enabled = 1;

        while (0 > linkkit_start(8, get_tsl_from_cloud, linkkit_loglevel_debug, &alink_ops, linkkit_cloud_domain_shanghai, sample_ctx))
        {
            LOGE("Call linkkit_start failed!!!\n");
        }

    #ifdef SERVICE_OTA_ENABLED
        linkkit_ota_init(fota_callback);
    #endif /* SERVICE_OTA_ENABLED */

        while (1) {
    #ifndef CM_SUPPORT_MULTI_THREAD
            linkkit_dispatch();
    #endif
            now = uptime_sec();
            if (prev_sec == now) {
    #ifdef CM_SUPPORT_MULTI_THREAD
                HAL_SleepMs(100);
    #else
                linkkit_yield(100);
    #endif /* CM_SUPPORT_MULTI_THREAD */
                continue;
            }

            /* about 30 seconds, assume trigger post property event about every 30s. */

    #ifdef POST_WIFI_STATUS
            if(now % 10 == 0) {
                post_property_wifi_status_once(sample_ctx);
            }
    #endif
            if (now % 30 == 0 && is_active(sample_ctx)) {
                post_all_prop(sample_ctx);
            }

            if (exit) break;

            /* after all, this is an sample, give a chance to return... */
            /* modify this value for this sample executaion time period */
            if (now > 60 * execution_time)
            {
                exit = 0;
            }

            if (bSend == 0)
            {
                sleep(2);
                NeedAuth(productKey, deviceKey, deviceSecret);
                bSend = 1;
            }

            prev_sec = now;
        }
    }while(0);


    (*env)->ReleaseStringUTFChars(env, pdKey_, pdKey);
    (*env)->ReleaseStringUTFChars(env, dvKey_, dvKey);
    (*env)->ReleaseStringUTFChars(env, dvSec_, dvSec);
    (*env)->ReleaseStringUTFChars(env, productKey_, productKey);
    (*env)->ReleaseStringUTFChars(env, deviceKey_, deviceKey);
    (*env)->ReleaseStringUTFChars(env, deviceSecret_, deviceSecret);
    (*env)->ReleaseStringUTFChars(env, ServerURL_, ServerURL);
    (*env)->ReleaseStringUTFChars(env, token_, token);
    pthread_mutex_destroy(&g_AddedUserInfoBuffLock);
    if (NULL != g_AddedUserInfoBuff)
    {
        free(g_AddedUserInfoBuff);
        g_AddedUserInfoBuff = NULL;
    }
    if (NULL != g_FaceDetectedData)
    {
        free(g_FaceDetectedData);
        g_FaceDetectedData = NULL;
    }
    if (NULL != g_FaceMatchedData)
    {
        free(g_FaceMatchedData);
         g_FaceMatchedData = NULL;
    }

	//linkkit_end();
    LOGI("out of sample!\n");
    return;
}

JNIEXPORT jstring JNICALL
Java_pc_tingyi_facedoor_FaceDoor_getLicenseData(JNIEnv *env, jobject instance) {
    return (*env)->NewStringUTF(env, gc_licenseData);
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_RefreashAddedUserInfo(JNIEnv *env, jobject instance,
                                                     jstring AddedUserInfo_) {
    pthread_t pid;
    LOGI("Java_pc_tingyi_facedoor_FaceDoor_UploadAddedUserInfo");
    const char *AddedUserInfo = (*env)->GetStringUTFChars(env, AddedUserInfo_, 0);

    FILE *p = NULL;

    pthread_mutex_lock(&g_AddedUserInfoBuffLock);
    if (NULL != g_AddedUserInfoBuff)
    {
        free(g_AddedUserInfoBuff);
        g_AddedUserInfoBuff = NULL;
    }

    g_AddedUserInfoBuff = (char *)malloc(strlen(AddedUserInfo)+1);
    if (NULL == g_AddedUserInfoBuff)
    {
        LOGE("Out of memory!\n");
        pthread_mutex_unlock(&g_AddedUserInfoBuffLock);
		(*env)->ReleaseStringUTFChars(env, AddedUserInfo_, AddedUserInfo);
        return;
    }
    bzero(g_AddedUserInfoBuff, strlen(AddedUserInfo)+1);
    strncpy(g_AddedUserInfoBuff, AddedUserInfo, strlen(AddedUserInfo));
    LOGI("g_AddedUserInfoBuff===>>>{%s}\n", g_AddedUserInfoBuff);
    pthread_mutex_unlock(&g_AddedUserInfoBuffLock);

    p = fopen("/sdcard/AddedUser", "w");

    if (p != NULL)
    {
        fputs(AddedUserInfo, p);
        fclose(p);
    }

    LOGI("AddedUserInfo_= %s\n", AddedUserInfo);

    // 调用HTTP2接口上传数据
    if (0 != pthread_create(&pid, NULL, thread1, "/sdcard/AddedUser"))
    {
        LOGE("thread1 create failed!\n");
    }

    (*env)->ReleaseStringUTFChars(env, AddedUserInfo_, AddedUserInfo);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    g_jvm = vm;

    LOGI("JNI_OnLoad");
    // AndroidJNI.GetVersion()
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_SetSyncRate(JNIEnv *env, jobject instance, jfloat rate) {
    g_SyncRate = rate;
    LOGI("g_SyncRate = %f\n", g_SyncRate);
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_OnMatched(JNIEnv *env, jobject instance, jstring facdID_, jfloat score, jstring facePicData_) {

    int img_len = 0;
    LOGI("Into Java_pc_tingyi_facedoor_FaceDoor_OnMatched\n");

    const char *facdID = (*env)->GetStringUTFChars(env, facdID_, 0);
    const char *facePicData = (*env)->GetStringUTFChars(env, facePicData_, 0);

    pthread_t pid;

    if (g_IsMatchedUploading == JNI_FALSE)
    {
        if (NULL == g_FaceMatchedData)
        {
            LOGE("ERR: g_FaceMatchedData is NULL!");
            return;
        }
        memset(g_FaceMatchedData, 0, g_outlen);
        img_len = ABase64_Decode(facePicData, g_FaceMatchedData, g_outlen);
        if (-1 == img_len)
        {
            LOGE("ERR: Faceimg is too big!");
            return;
        }

        g_IsMatchedUploading = JNI_TRUE;

        FILE *p = NULL;
        p = fopen("/sdcard/onMatched.jpg", "w");

        if (NULL != p)
        {
            fwrite(g_FaceMatchedData, img_len, 1, p);
            fclose(p);
        }

        // 调用HTTP2接口上传数据
        if (0 != pthread_create(&pid, NULL, thread2, "/sdcard/onMatched.jpg"))
        {
            LOGE("pthread_create1 failed!\n");
        }

        memset(g_faceID, 0 , sizeof(g_faceID));
        strncpy(g_faceID, facdID, strlen(facdID));
        g_score = score;
    }
    else
    {
        LOGI("OnMatched faceimg is uploading!\n");
    }

    (*env)->ReleaseStringUTFChars(env, facdID_, facdID);
    (*env)->ReleaseStringUTFChars(env, facePicData_, facePicData);

    pthread_detach(pid);
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_OnDetected(JNIEnv *env, jobject instance, jstring facePicData_) {
    const char *facePicData = (*env)->GetStringUTFChars(env, facePicData_, 0);
    int img_len = 0;

    LOGI("Into Java_pc_tingyi_facedoor_FaceDoor_OnDetected\n");

    pthread_t pid;

    if (g_IsDetectedUploading == JNI_FALSE)
    {
        if (NULL == g_FaceDetectedData)
        {
            LOGE("ERR: g_FaceDetectedData is NULL!");
            return;
        }
        memset(g_FaceDetectedData, 0, g_outlen);
        img_len = ABase64_Decode(facePicData, g_FaceDetectedData, g_outlen);
        if (-1 == img_len)
        {
            LOGE("ERR: Faceimg is too big!");
            return;
        }

        g_IsDetectedUploading = JNI_TRUE;

        FILE *p = NULL;
        p = fopen("/sdcard/onDetected.jpg", "w");

        if (NULL != p)
        {
            //fputs(g_FaceImgData, p);
            fwrite(g_FaceDetectedData, img_len, 1, p);
            fclose(p);
        }

        // 调用HTTP2接口上传数据
        if (0 != pthread_create(&pid, NULL, thread2FD, "/sdcard/onDetected.jpg"))
        {
            LOGE("pthread_create1 failed!\n");
        }
    }
    else
    {
        LOGI("OnDetected faceimg is uploading!\n");
    }

    (*env)->ReleaseStringUTFChars(env, facePicData_, facePicData);

    pthread_detach(pid);
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_SetGroupID(JNIEnv *env, jobject instance, jstring groupID_) {
    const char *groupID = (*env)->GetStringUTFChars(env, groupID_, 0);

    strncpy(g_stFaceDoorProperty.acFaceGroupID, groupID, sizeof(g_stFaceDoorProperty.acFaceGroupID));
    strcat(g_stFaceDoorProperty.acFaceGroupID, "\n");
    write_property();
    LOGI("g_sample_context.GroupID:%s!\n", g_stFaceDoorProperty.acFaceGroupID);
    (*env)->ReleaseStringUTFChars(env, groupID_, groupID);
}

JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_VerifySDKNeedAuth(JNIEnv *env, jobject instance,
                                                   jstring PackageName_, jstring ClientID_,
                                                   jstring PublicKey_, jstring Token_) {
    const char *PackageName = (*env)->GetStringUTFChars(env, PackageName_, 0);
    const char *ClientID = (*env)->GetStringUTFChars(env, ClientID_, 0);
    const char *PublicKey = (*env)->GetStringUTFChars(env, PublicKey_, 0);
    const char *Token = (*env)->GetStringUTFChars(env, Token_, 0);

    // TODO
    strncpy(g_PackageName, PackageName, sizeof(g_PackageName));
    strncpy(g_Me, PublicKey, sizeof(g_Me));
    strncpy(g_ClientId, ClientID, sizeof(g_ClientId));
    strncpy(g_Token, Token, sizeof(g_Token));

    NeedAuth(g_PackageName, g_Me, g_ClientId);

    (*env)->ReleaseStringUTFChars(env, PackageName_, PackageName);
    (*env)->ReleaseStringUTFChars(env, ClientID_, ClientID);
    (*env)->ReleaseStringUTFChars(env, PublicKey_, PublicKey);
    (*env)->ReleaseStringUTFChars(env, Token_, Token);


}


JNIEXPORT void JNICALL
Java_pc_tingyi_facedoor_FaceDoor_setDeviceInfoV2(JNIEnv *env, jobject instance, jstring pdKey_,
                                                 jstring dvKey_, jstring dvSec_, jstring Token_,
                                                 jstring ServerURL_, jint ServerPort,
                                                 jstring DeviceEncryptTmp_,
                                                 jobject MessageCallbackTmp) {
    sample_context_t* sample_ctx = &g_sample_context;
    int execution_time = 0;
    int get_tsl_from_cloud = 1;
    int exit = 0;
    int bSend = 0;
    pthread_t  pid;
    unsigned long long now = 0;
    unsigned long long prev_sec = 0;
    fpos_t fpos; //当前位置
    int    fileLenth = 0;
    const char *pdKey = (*env)->GetStringUTFChars(env, pdKey_, 0);
    const char *dvKey = (*env)->GetStringUTFChars(env, dvKey_, 0);
    const char *dvSec = (*env)->GetStringUTFChars(env, dvSec_, 0);
    const char *Token = (*env)->GetStringUTFChars(env, Token_, 0);
    const char *ServerURL = (*env)->GetStringUTFChars(env, ServerURL_, 0);
    const char *DeviceEncrypt = (*env)->GetStringUTFChars(env, DeviceEncryptTmp_, 0);

    strncpy(g_ProductKey, pdKey, sizeof(g_ProductKey));
    strncpy(g_DeviceName, dvKey, sizeof(g_DeviceName));
    strncpy(g_DeviceSecret, dvSec, sizeof(g_DeviceSecret));
    strncpy(g_ServerURL, ServerURL, sizeof(g_ServerURL));
    strncpy(g_Token, Token, sizeof(g_Token));
    strncpy(g_DeviceEncrypt, DeviceEncrypt, sizeof(g_DeviceEncrypt));
    g_ServerPort = ServerPort;

    do
    {
        pthread_mutex_init(&g_AddedUserInfoBuffLock, NULL);

        if (NULL == g_AddedUserInfoBuff)
        {
            FILE *p = fopen("/sdcard/AddedUser", "rb");

            if (p != NULL)
            {
                fgetpos(p, &fpos); //获取当前位置
                fseek(p, 0, SEEK_END);
                fileLenth = ftell(p);
                fsetpos(p,&fpos); //恢复之前的位置

                g_AddedUserInfoBuff = (char *)malloc(fileLenth+1);
                if (g_AddedUserInfoBuff == NULL)
                {
                    LOGE("No Memory!\n");
                    fclose(p);
                    break;
                }
                bzero(g_AddedUserInfoBuff, fileLenth+1);

                fgets(g_AddedUserInfoBuff, fileLenth, p);
                fclose(p);
            }

        }

        if (NULL == g_FaceDetectedData)
        {
            g_FaceDetectedData = (char *) malloc(g_outlen);

            if (NULL == g_FaceDetectedData)
            {
                LOGE("ERR: Malloc mem failed!\n");
            }
        }

        if (NULL == g_FaceMatchedData)
        {
            g_FaceMatchedData = (char*)malloc(g_outlen);

            if (NULL == g_FaceMatchedData)
            {
                LOGE("ERR: Malloc mem failed!\n");
            }
        }



        //读取property属性文件
        read_property();

        (*env)->GetJavaVM(env, &g_jvm);
        g_obj = (*env)->NewGlobalRef(env, instance);

        //起HTTP2线程
        if (0 != pthread_create(&pid, NULL, thread_http2, NULL))
        {
            LOGE("thread_http2 create failed!\n");
            break;
        }
        execution_time = execution_time < 1 ? 1 : execution_time;
        LOGI("sample execution time: %d minutes\n", execution_time);
        LOGI("%s tsl from cloud\n", get_tsl_from_cloud == 0 ? "Not get" : "get");
        HAL_SetProductKey(pdKey);
        HAL_SetDeviceName(dvKey);
        HAL_SetDeviceSecret(dvSec);

        memset(sample_ctx, 0, sizeof(sample_context_t));
        sample_ctx->thing_enabled = 1;

        while (0 > linkkit_start(8, get_tsl_from_cloud, linkkit_loglevel_debug, &alink_ops, linkkit_cloud_domain_shanghai, sample_ctx))
        {
            LOGE("Call linkkit_start failed!!!\n");
        }

#ifdef SERVICE_OTA_ENABLED
        linkkit_ota_init(fota_callback);
#endif /* SERVICE_OTA_ENABLED */

        while (1) {
#ifndef CM_SUPPORT_MULTI_THREAD
            linkkit_dispatch();
#endif
            now = uptime_sec();
            if (prev_sec == now) {
#ifdef CM_SUPPORT_MULTI_THREAD
                HAL_SleepMs(100);
#else
                linkkit_yield(100);
#endif /* CM_SUPPORT_MULTI_THREAD */
                continue;
            }

            /* about 30 seconds, assume trigger post property event about every 30s. */

#ifdef POST_WIFI_STATUS
            if(now % 10 == 0) {
                post_property_wifi_status_once(sample_ctx);
            }
#endif
            if (now % 30 == 0 && is_active(sample_ctx)) {
                post_all_prop(sample_ctx);
            }

            if (exit) break;

            /* after all, this is an sample, give a chance to return... */
            /* modify this value for this sample executaion time period */
            if (now > 60 * execution_time)
            {
                exit = 0;
            }

            if (bSend == 0)
            {
                sleep(2);
                LOGI("Begin to Call NeedAuth~~~");
                NeedAuthV2(g_ProductKey, g_DeviceName, g_DeviceSecret);
                bSend = 1;
            }

            prev_sec = now;
        }
    }while(0);

    (*env)->ReleaseStringUTFChars(env, pdKey_, pdKey);
    (*env)->ReleaseStringUTFChars(env, dvKey_, dvKey);
    (*env)->ReleaseStringUTFChars(env, dvSec_, dvSec);
    (*env)->ReleaseStringUTFChars(env, Token_, Token);
    (*env)->ReleaseStringUTFChars(env, ServerURL_, ServerURL);
    (*env)->ReleaseStringUTFChars(env, DeviceEncryptTmp_, DeviceEncrypt);
}