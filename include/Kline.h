/**
 * @file   Kline.cpp
 * @author Ye Shiwei <yeshiwei.math@gmail.com>
 * @date   Sun Jun 24 20:01:11 2018
 * 
 * @brief  K线类，根据插入的tick数据生成K线。
 * 
 * 
 */

#ifndef __KLINE_H__
#define __KLINE_H__

#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"

#define MAX_PRICE 1e10

class KlineElement{
public:

  /** 
   * 创建一根K线.
   * 
   * @param tick 
   * @param UpdateTimet 
   */
  KlineElement(CThostFtdcDepthMarketDataField *tick,
	       TThostFtdcTimeType UpdateTimet) {
    OpenPrice = HighestPrice = LowestPrice = ClosePrice = tick -> LastPrice;
    strcpy(TradingDay, tick -> TradingDay);
    strcpy(UpdateTime, UpdateTimet);
  }

  /** 
   * 根据新tick数据更新K线.
   * 
   * @param tick 
   */
  void update_tick(CThostFtdcDepthMarketDataField *tick) {
    if ( tick -> LastPrice < MAX_PRICE
	 && tick -> LastPrice > 0) {
      HighestPrice = std::max(tick -> LastPrice, HighestPrice);
      LowestPrice = std::min(tick -> LastPrice, LowestPrice);
      ClosePrice = tick -> LastPrice;
    }
  }

  std::string to_string() {
    std::ostringstream strs;
    strs << TradingDay << ","
	 << UpdateTime << ","
	 << OpenPrice << ","
	 << HighestPrice << ","
	 << LowestPrice << ","
	 << ClosePrice;
    return strs.str();
  }
  
  double OpenPrice;
  double HighestPrice;
  double LowestPrice;
  double ClosePrice;
  TThostFtdcDateType TradingDay;
  TThostFtdcTimeType UpdateTime;
};


class Kline{
public:
  
  /** 
   *  创建一个K线。
   * 
   * @param tick 
   */
  Kline (CThostFtdcDepthMarketDataField *tick) {
    TThostFtdcTimeType UpdateTime;
    PeriodofTick(UpdateTime, tick);
    KlineElement element(tick, UpdateTime);
    elements.push_back(element);
    char dir[100];
    sprintf(dir, "Kline/%s/", tick -> TradingDay);
    char filen[100];
    sprintf(filen,"%s%s-%s.txt", dir, tick -> TradingDay,
	    tick -> InstrumentID);
    mkdir(dir, S_IRWXU);
    filename = filen;
    std::cout << filename << std::endl;
  }
  
  /** 
   * 计算一个tick数据所在的K线时间。
   * 
   * @param KlineUpdatetime 输出
   * @param tick 
   */
  void PeriodofTick(TThostFtdcTimeType KlineUpdatetime,
		    CThostFtdcDepthMarketDataField *tick) {
    int hour = (tick -> UpdateTime[0] - '0') * 10 + tick -> UpdateTime[1] - '0';
    int minute = (tick -> UpdateTime[3] - '0') * 10 + tick -> UpdateTime[4] - '0';
    int second = (tick -> UpdateTime[6] - '0') * 10 + tick -> UpdateTime[7] - '0';
    int millisec = tick -> UpdateMillisec;
    if ( second != 0 || millisec != 0) {
      minute += 1;
      if ( minute == 60 ) {
	minute = 0;
	hour += 1;
	if (hour == 24) {
	  hour = 0;
	}
      }
    }
    sprintf(KlineUpdatetime, "%02d:%02d:00", hour, minute);
  }

  /** 
   * 输出最后一根K线.
   * 
   */
  void output_back() {
      std::ofstream OutFile;
      OutFile.open(filename.c_str(), std::ofstream::out | std::ofstream::app);
      std::string res = elements.back().to_string();
      std::cout << res << std::endl;
      OutFile << res << std::endl;
      OutFile.close();
  }

  /** 
   * 往K线中添加一个tick, 如果属于最近一根K线，更新K线，否则添加一根K线。
   * 
   * @param tick 
   */
  void add_tick(CThostFtdcDepthMarketDataField *tick) {
    TThostFtdcTimeType UpdateTime;
    PeriodofTick(UpdateTime, tick);
    if ( strcmp(UpdateTime, elements.back().UpdateTime) == 0 ) {
      elements.back().update_tick(tick);
    } else {
      output_back();
      KlineElement element(tick, UpdateTime);
      elements.push_back(element);
    }
  }

private:
  std::string filename;
  std::vector<KlineElement> elements;
};

#endif
