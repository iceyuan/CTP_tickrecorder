//简单的例子，介绍CThostFtdcMdApi和CThostFtdcMdSpi接口的使用。
// 本例将演示一个报单录入操作的过程
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <sys/stat.h>

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "util.h"
using namespace std;

// 会员代码
TThostFtdcBrokerIDType g_chBrokerID="9999";
// 交易用户代码
TThostFtdcUserIDType g_chUserID="090419";
// 密码
TThostFtdcPasswordType	Password;
//  char TradeFrontAddr[] ="tcp://180.168.146.187:10010";
char TradeFrontAddr[] ="tcp://180.168.146.187:10030";

//  char TradeFrontAddr[] ="tcp://180.168.146.187:10000";
char MarketFrontAddr[] ="tcp://180.168.146.187:10031";

class TSimpleHandler : public CThostFtdcTraderSpi
{
public:
  // 构造函数，需要一个有效的指向CThostFtdcMduserApi实例的指针
  TSimpleHandler(CThostFtdcTraderApi *pUserApi, CThostFtdcMdApi *pUserApim) :
    m_pUserApi(pUserApi), mm_pUserApi(pUserApim) {}
  ~TSimpleHandler() {}
  // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
  virtual void OnFrontConnected()
  {
    CThostFtdcReqUserLoginField reqUserLogin;
    // get BrokerID
    strcpy(reqUserLogin. BrokerID, g_chBrokerID);
    // get userid
    strcpy(reqUserLogin.UserID, g_chUserID);
    // get password
    strcpy(reqUserLogin.Password, Password);
    // 发出登陆请求
    m_pUserApi->ReqUserLogin(&reqUserLogin, 0);
  }
  // 当客户端与交易托管系统通信连接断开时，该方法被调用
  virtual void OnFrontDisconnected(int nReason)
  {
    // 当发生这个情况后，API会自动重新连接，客户端可不做处理
    printf("OnFrontDisconnected.\n");
  }
  // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField
			      *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool
			      bIsLast)
  {
    printf("Trade OnRspUserLogin:\n");
    char Msg[200];
    code_convert(pRspInfo -> ErrorMsg, strlen(pRspInfo->ErrorMsg), Msg, 200);
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n",
	   pRspInfo->ErrorID, Msg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
      // 端登失败，客户端需进行错误处理
      printf("Failed to login, errorcode=%d errormsg=%s requestid=%d chain=%d", pRspInfo->ErrorID, pRspInfo->ErrorMsg,
	     nRequestID, bIsLast);
      exit(-1);
    }

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    m_pUserApi->ReqQryInstrument(&req, ++iRequestID);
  }
  // 报单录入应答
  virtual void OnRspOrderInsert(CThostFtdcInputOrderField
				*pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool
				bIsLast)
  {
    // 输出报单录入结果
    char Msg[200];
    code_convert(pRspInfo -> ErrorMsg, strlen(pRspInfo->ErrorMsg), Msg, 200);
    printf("OnRspOrderInsert: ErrorCode=[%d], ErrorMsg=[%s]\n",
	   pRspInfo->ErrorID, Msg);
    // 通知报单录入完成
    //SetEvent(g_hEvent);
  };
  ///报单回报
  virtual void OnRtnOrder(CThostFtdcOrderField *pOrder)
  {
    printf("OnRtnOrder:\n");
    char Msg[200];
    code_convert(pOrder -> StatusMsg, strlen(pOrder -> StatusMsg), Msg, 200);
    printf("OrderSysID=[%s], StatusMsg=[%s]\n", pOrder->OrderSysID, Msg);

  }
  // 针对用户请求的出错通知
  virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int
			  nRequestID, bool bIsLast) {
    printf("OnRspError:\n");
    char Msg[200];
    code_convert(pRspInfo->ErrorMsg, strlen(pRspInfo->ErrorMsg), Msg, 200);
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n",
	   pRspInfo->ErrorID, Msg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    // 客户端需进行错误处理
    //{客户端的错误处理}
  }

  virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf("OnRspQryInstrument:\n");
    printf("InstrumentID[%s], ExchangeID[%s], ExchangeInstID[%s],ProductID[%s].\n",
	   pInstrument->InstrumentID,
	   pInstrument->ExchangeID,
	   pInstrument->ExchangeInstID,
	   pInstrument->ProductID);
    char *Instrument[] = {pInstrument->InstrumentID};
    mm_pUserApi->SubscribeMarketData (Instrument,1);
  }

private:
  // 指向CThostFtdcMduserApi实例的指针
  CThostFtdcTraderApi *m_pUserApi;
  // 指向CThostFtdcMdApi实例的指针
  CThostFtdcMdApi *mm_pUserApi;
  int iRequestID;
};

class CSimpleHandler : public CThostFtdcMdSpi
{
public:
  // 构造函数，需要一个有效的指向CThostFtdcMdApi实例的指针
  CSimpleHandler(CThostFtdcMdApi *pUserApi) : m_pUserApi(pUserApi) {}
  ~CSimpleHandler() {}
  // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
  virtual void OnFrontConnected()
  {
    CThostFtdcReqUserLoginField reqUserLogin;
    // get BrokerID
    //printf("BrokerID:");
    //scanf("%s",&g_chBrokerID);
    strcpy(reqUserLogin. BrokerID, g_chBrokerID);
    // get userid
    //printf("userid:");
    //scanf("%s", &g_chUserID);
    strcpy(reqUserLogin.UserID, g_chUserID);
    // get password
    printf("password:");
    scanf("%s", Password);
    strcpy(reqUserLogin.Password, Password);
    // 发出登陆请求
    m_pUserApi->ReqUserLogin(&reqUserLogin, 0);
  }
  // 当客户端与交易托管系统通信连接断开时，该方法被调用
  virtual void OnFrontDisconnected(int nReason)
  {
    // 当发生这个情况后，API会自动重新连接，客户端可不做处理
    printf("OnFrontDisconnected.\n");
  }
  // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
			      CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    printf("OnRspUserLogin:\n");
    char Msg[200];
    code_convert(pRspInfo -> ErrorMsg, strlen(pRspInfo->ErrorMsg), Msg, 200);

    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,
	   Msg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
      // 端登失败，客户端需进行错误处理
      printf("Failed to login, errorcode=%d errormsg=%s requestid=%d chain=%d",
	     pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
      exit(-1);
    }
    // 端登成功
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // print various components of tm structure.
    cout << "Year" << 1970 + ltm->tm_year<<endl;
    cout << "Month: "<< 1 + ltm->tm_mon<< endl;
    cout << "Day: "<<  ltm->tm_mday << endl;
    cout << "Time: "<< ltm->tm_hour << ":";
    cout << 1 + ltm->tm_min << ":";
    cout << 1 + ltm->tm_sec << endl;
    if (ltm->tm_hour >= 17 ) {
      ltm -> tm_mday +=1;
      now = mktime(ltm);
      ltm = localtime(&now);
    } 
    char dir[100];
    sprintf(dir, "TickData/%d%02d/",1900 + ltm->tm_year, 1 + ltm->tm_mon);
    char filename[100];
    sprintf(filename,"%s/%d%02d%02d.txt", dir, 1900+ ltm->tm_year,
	    1 + ltm->tm_mon, ltm->tm_mday);
    mkdir(dir, S_IRWXU);
    OutFile.open(filename, ofstream::out | ofstream::app);
    /**/
    pid_t pid = fork();
    if (pid == 0) {
      // 产生一个CThostFtdcTraderApi实例
      CThostFtdcTraderApi *pUserApi =
      CThostFtdcTraderApi::CreateFtdcTraderApi();
      // 产生一个事件处理的实例
      TSimpleHandler sh(pUserApi, m_pUserApi);
      // 注册一事件处理的实例
      pUserApi->RegisterSpi(&sh);
      // 订阅私有流
      // TERT_RESTART:从本交易日开始重传
      // TERT_RESUME:从上次收到的续传
      // TERT_QUICK:只传送登录后私有流的内容
      pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);
      // 订阅公共流
      // TERT_RESTART:从本交易日开始重传
      // TERT_RESUME:从上次收到的续传
      // TERT_QUICK:只传送登录后公共流的内容
      pUserApi->SubscribePublicTopic(THOST_TERT_RESUME);
      // 设置交易托管系统服务的地址，可以注册多个地址备用
      pUserApi->RegisterFront(TradeFrontAddr);
      // 使客户端开始与后台服务建立连接
      pUserApi->Init();
      // 释放API实例
      pUserApi->Release();
    }
    /**/
  }

  // 行情应答
  virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField
				    *pDepthMarketData)
  {
    // 输出报单录入结果
    //printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,
    //pRspInfo->ErrorMsg);
    printf("TradingDay=[%s], Instrument=[%s], Time[%s:%d], LastPrice=[%lf]\n",
	   pDepthMarketData->TradingDay,
	   pDepthMarketData->InstrumentID,
	   pDepthMarketData->UpdateTime,
	   pDepthMarketData->UpdateMillisec,
	   pDepthMarketData->LastPrice);
    ///交易日
    OutFile << pDepthMarketData -> TradingDay << "|"
      ///合约代码
	    << pDepthMarketData -> InstrumentID << "|"
      ///交易所代码
	    << pDepthMarketData -> ExchangeID << "|"
      ///合约在交易所的代码
	    << pDepthMarketData -> ExchangeInstID << "|"
      ///最新价
	    << pDepthMarketData -> LastPrice << "|"
	///上次结算价
	    << pDepthMarketData -> PreSettlementPrice << "|"
	///昨收盘
	    << pDepthMarketData -> PreClosePrice << "|"
	///昨持仓量
	    << pDepthMarketData -> PreOpenInterest << "|"
	///今开盘
	    << pDepthMarketData -> OpenPrice << "|"
	///最高价
	    << pDepthMarketData -> HighestPrice << "|"
	///最低价
	    << pDepthMarketData -> LowestPrice << "|"
	///数量
	    << pDepthMarketData -> Volume << "|"
	///成交金额
	    << pDepthMarketData -> Turnover << "|"
	///持仓量
	    << pDepthMarketData -> OpenInterest << "|"
	///今收盘
	    << pDepthMarketData -> ClosePrice << "|"
	///本次结算价
	    << pDepthMarketData -> SettlementPrice << "|"
	///涨停板价
	    << pDepthMarketData -> UpperLimitPrice << "|"
	///跌停板价
	    << pDepthMarketData -> LowerLimitPrice << "|"
	///昨虚实度
	    << pDepthMarketData -> PreDelta << "|"
	///今虚实度
	    << pDepthMarketData -> CurrDelta << "|"
	///最后修改时间
	    << pDepthMarketData -> UpdateTime << "|"
	///最后修改毫秒
	    << pDepthMarketData -> UpdateMillisec << "|"
	///申买价一
	    << pDepthMarketData -> BidPrice1 << "|"
	///申买量一
	    << pDepthMarketData -> BidVolume1 << "|"
	///申卖价一
	    << pDepthMarketData -> AskPrice1 << "|"
	///申卖量一
	    << pDepthMarketData -> AskVolume1 << "|"
	///申买价二
	    << pDepthMarketData -> BidPrice2 << "|"
	///申买量二
	    << pDepthMarketData -> BidVolume2 << "|"
	///申卖价二
	    << pDepthMarketData -> AskPrice2 << "|"
	///申卖量二
	    << pDepthMarketData -> AskVolume2 << "|"
	///申买价三
	    << pDepthMarketData -> BidPrice3 << "|"
	///申买量三
	    << pDepthMarketData -> BidVolume3 << "|"
	///申卖价三
	    << pDepthMarketData -> AskPrice3 << "|"
	///申卖量三
	    << pDepthMarketData -> AskVolume3 << "|"
	///申买价四
	    << pDepthMarketData -> BidPrice4 << "|"
	///申买量四
	    << pDepthMarketData -> BidVolume4 << "|"
	///申卖价四
	    << pDepthMarketData -> AskPrice4 << "|"
	///申卖量四
	    << pDepthMarketData -> AskVolume4 << "|"
	///申买价五
	    << pDepthMarketData -> BidPrice5 << "|"
	///申买量五
	    << pDepthMarketData -> BidVolume5 << "|"
	///申卖价五
	    << pDepthMarketData -> AskPrice5 << "|"
	///申卖量五
	    << pDepthMarketData -> AskVolume5 << "|"
	///当日均价
	    << pDepthMarketData -> AveragePrice << "|"
	///业务日期
	    << pDepthMarketData -> ActionDay
	    << endl;
  };

  // 针对用户请求的出错通知
  virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID,
			  bool bIsLast) {
    printf("OnRspError:\n");
    char Msg[200];
    code_convert(pRspInfo->ErrorMsg, strlen(pRspInfo->ErrorMsg), Msg, 200);
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,
	   Msg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    // 客户端需进行错误处理
    //{客户端的错误处理}
  }
private:
  // 指向CThostFtdcMdApi实例的指针
  CThostFtdcMdApi *m_pUserApi;
  int iRequestID;
  ofstream OutFile;
};

void parse_config(string config_file) {
  ifstream inFile;
  inFile.open(config_file.c_str());
  if (!inFile) {
    cout << "Unable to Open file:" << config_file << endl;
  }
  string ParmName, Parm;
  while (inFile >> ParmName) {
    if (ParmName.compare("BrokerID")==0) {
      inFile >> g_chBrokerID;;
    } else if (ParmName.compare("UserID")==0) {
      inFile >> g_chUserID;
    } else if (ParmName.compare("TradeFront")==0) {
      inFile >> TradeFrontAddr;
    } else if (ParmName.compare("MarketFront")==0) {
      inFile >> MarketFrontAddr;
    } else {
      string buf;
      inFile >> buf;
      cout << "unknow parameter name:" << ParmName << endl;
    }
  }
}

int main(int argc, char **argv)
{
  
  string config_file = "config";
  if (argc > 1) {
    config_file = argv[1];
  }
  parse_config(config_file);
  
  // 产生一个CThostFtdcMdApi实例
  CThostFtdcMdApi *pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
  // 产生一个事件处理的实例
  CSimpleHandler sh(pUserApi);
  // 注册一事件处理的实例
  pUserApi->RegisterSpi(&sh);
  // 设置交易托管系统服务的地址，可以注册多个地址备用
  pUserApi->RegisterFront(MarketFrontAddr);
  // 使客户端开始与后台服务建立连接
  pUserApi->Init();
  // 客户端等待报单操作完成
  //WaitForSingleObject(g_hEvent, INFINITE);
  // 释放API实例
  pUserApi->Release();
  return 0;
}

