package pc.tingyi.facedoor;

import android.util.Log;

/**
 * FaceDoor方法简述.
 * <p>主要完成打通设备端侧与服务端物模型消息交互的功能<br>
 * @version 1.0.0
 */
public class FaceDoor extends Thread
{
    static
    {
        System.loadLibrary("facedoor");
    }

    String pdKeyTmp;
    String dvKeyTmp;
    String dvSecTmp;
    String PackageNameTmp;
    String ClientIDTmp;
    String PublicKeyTmp;
    String TokenTmp;
    String ServerURLTmp;
    String DeviceEncryptTmp;
    int ServerPortTmp;
    MessageCallback MessageCallbackTmp;

    /**
     * <p> 设置设备启动时的初始化参数
     * @param pdKey 设备三元组---产品Key
     * @param dvKey 设备三元组---设备名称
     * @param dvSec 设备三元组---设备秘钥
     * @param PackageName VerifySDK包名
     * @param ClientID    VerifySDK设备序列号
     * @param PublicKey   VerifySDK公钥
     * @param ServerURL   Http2服务器域名(默认为".iot-as-http2.cn-shanghai.aliyuncs.com")
     * @param ServerPort  Http2服务器端口(默认为0)
     * @param MessageCallback  iotkit消息回调接口
     */
    public void API_setLocalVariables(String pdKey, String dvKey, String dvSec,
                                      String PackageName, String ClientID, String PublicKey,
                                      String Token,
                                      String ServerURL, int ServerPort,
                                      MessageCallback MessageCallback)
    {
        pdKeyTmp = pdKey;
        dvKeyTmp = dvKey;
        dvSecTmp = dvSec;
        PackageNameTmp = PackageName;
        ClientIDTmp = ClientID;
        PublicKeyTmp = PublicKey;
        TokenTmp = Token;
        ServerURLTmp = ServerURL;
        ServerPortTmp = ServerPort;
        MessageCallbackTmp = MessageCallback;

        return;
    }

    public void API_setLocalVariablesV2(String pdKey, String dvKey, String dvSec,
                                        String Token,String ServerURL, int ServerPort, 
										String DeviceEncrypt,
                                        MessageCallback MessageCallback)
    {
        pdKeyTmp = pdKey;
        dvKeyTmp = dvKey;
        dvSecTmp = dvSec;
        TokenTmp = Token;
        ServerURLTmp = ServerURL;
        DeviceEncryptTmp = DeviceEncrypt;
        ServerPortTmp = ServerPort;
        MessageCallbackTmp = MessageCallback;

        return;
    }

    /**
     * <p> 向服务端上报'设备需授权'事件
     */
    public void API_VerifySDKNeedAuth(String PackageName, String ClientID, String PublicKey,
                                      String Token)
    {
        VerifySDKNeedAuth(PackageName, ClientID, PublicKey, Token);
    }

    /**
     * <p> 更新'已添加的人脸'数据文件(/sdcard/AddedUser)
     * @param AddedUserInfo 本次添加的人脸集合
     */
    public void API_RefreashAddedUserInfo(String AddedUserInfo)
    {
        RefreashAddedUserInfo(AddedUserInfo);
    }

    /**
     * <p> 设置同步人脸进度百分比
     * @param rate 同步人脸进度
     */
    public void API_SetSyncRate(float rate)
    {
        SetSyncRate(rate);
    }

    /**
     * <p> 人脸比对事件上报
     * @param facdID 人脸图ID
     * @param score 人脸图比对得分
     * @param facePicData 人脸图数据(需将人脸图原数据Base64编码)
     */
    public void API_OnMatched(String facdID, float score, String facePicData)
    {
        OnMatched(facdID, score, facePicData);
    }

    /**
     * <p> 人脸检测事件上报
     * @param facePicData 人脸图数据(需将人脸图原数据Base64编码)
     */
    public void API_OnDetected(String facePicData)
    {
        OnDetected(facePicData);
    }

    public void API_SetGroupID(String groupID)
    {
        SetGroupID(groupID);
    }

    /**
     * <p> iotkit消息回调接口
     */
    public interface MessageCallback
    {
        /**
         * <p> VerifySDK授权文件下发事件回调
         * * @param licenseData 授权文件
         * * @param statusCode 授权状态码
         * * @param statusCodeDescription 授权状态码描述
         */
        public void licenseDataCB(String licenseData, int statusCode, String statusCodeDescription);

        /**
         * <p> 批量更新人脸库图片消息回调
         * * @param FacePicturesUrl 图片文件URL
         */
        public void downloadSyncFacePictures(String FacePicturesUrl);
    }



    public int init()
    {
        return 0;
    }

    /*======消息回调接口======*/
    //设备license授权
    public void OnMessage(int statusCode, String licenseData, String statusDesc)
    {
        MessageCallbackTmp.licenseDataCB(licenseData, statusCode, statusDesc);
    }

    //批量布控人脸图
    public void OnSyncFacePictures(String FacePicturesUrl)
    {
        SyncFacePicturesThread myThread = new SyncFacePicturesThread(FacePicturesUrl, this);

        myThread.start();
    }
    /*=======================*/

    //设置设备三元组
    private native void setDeviceInfo(String pdKey, String dvKey, String dvSec,
                                      String PackageName, String ClientID, String PublicKey,
                                      String Token, String ServerURL, int ServerPort,
                                      MessageCallback MessageCallbackTmp);
									  
    //设置设备三元组(携带设备可逆加密信息) 
    private native void setDeviceInfoV2(String pdKey, String dvKey, String dvSec,
                                        String Token, String ServerURL, int ServerPort,
                                        String DeviceEncryptTmp,
                                        MessageCallback MessageCallbackTmp);

    //VerifySDK授权
    private native void VerifySDKNeedAuth(String PackageName, String ClientID, String PublicKey,
                                          String Token);

    //上传添加成功的人脸图信息
    private native void RefreashAddedUserInfo(String AddedUserInfo);

    //
    private native void SetSyncRate(float rate);

    //人脸比对事件上报
    private native void OnMatched(String facdID, float score, String facePicData);

    //人脸检测事件上报
    private native void OnDetected(String facePicData);

    //设置人脸组ID
    private native void SetGroupID(String groupID);

    public void run()
    {
        if ((null != DeviceEncryptTmp && DeviceEncryptTmp.length() > 0))
        {
            setDeviceInfoV2(pdKeyTmp, dvKeyTmp, dvSecTmp, TokenTmp, ServerURLTmp, ServerPortTmp,
                            DeviceEncryptTmp,MessageCallbackTmp);
        }
        else
        {
            setDeviceInfo(pdKeyTmp, dvKeyTmp, dvSecTmp, PackageNameTmp, ClientIDTmp, PublicKeyTmp,
                    TokenTmp, ServerURLTmp, ServerPortTmp,MessageCallbackTmp);
        }
    }

    public class SyncFacePicturesThread extends Thread {
        String url;
        FaceDoor faceDoor;

        public SyncFacePicturesThread(String url, FaceDoor faceDoor) {
            this.url = url;
            this.faceDoor = faceDoor;
        }

        public void run(){
            faceDoor.MessageCallbackTmp.downloadSyncFacePictures(url);

            System.out.println("MyThread running");
        }
    }
}
