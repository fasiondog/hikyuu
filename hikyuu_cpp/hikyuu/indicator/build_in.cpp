#include "build_in.h"

#include "hikyuu/part/PartRegister.h"
#include "hikyuu/part/PartRegistration.h"

namespace hku {

void registerPart_LAST() {
    registerIndicatorPart("core.ind.last", [] { return LAST(); });
}

void registerPart_AVEDEV() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "AVEDEV";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "22"});
    registerIndicatorPart("core.ind.avedev", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return AVEDEV(std::get<Indicator>(pp.at("ind").at(0)),
                                            static_cast<int>(sc.get<int64_t>("n")));
                          });
}

void registerPart_BETWEEN() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "BETWEEN";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "z", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.between", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return BETWEEN(std::get<Indicator>(pp.at("x").at(0)),
                                             std::get<Indicator>(pp.at("y").at(0)),
                                             std::get<Indicator>(pp.at("z").at(0)));
                          });
}

void registerPart_CROSS() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CROSS";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.cross", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return CROSS(std::get<Indicator>(pp.at("x").at(0)),
                                           std::get<Indicator>(pp.at("y").at(0)));
                          });
}

void registerPart_DOWNNDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "DOWNNDAY";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.downnday", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return DOWNNDAY(std::get<Indicator>(pp.at("ind").at(0)),
                                              static_cast<int>(sc.get<int64_t>("n")));
                          });
}

void registerPart_ICIR() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "ICIR";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "stks", .type = "StockList", .required = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "1"});
    meta.params.push_back(ParamMeta{.name = "rolling_n", .type = "int", .default_value = "120"});
    meta.params.push_back(ParamMeta{.name = "spearman", .type = "bool", .default_value = "true"});
    meta.params.push_back(ParamMeta{.name = "strict", .type = "bool", .default_value = "false"});
    registerIndicatorPart(
      "core.ind.icir", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          HKU_THROW_EXCEPTION(std::logic_error,
                              "ICIR can not be created from the registry, it needs a StockList");
          return Indicator();
      });
}

void registerPart_IR() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "IR";
    meta.params.push_back(
      ParamMeta{.name = "p", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "b", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "100"});
    registerIndicatorPart(
      "core.ind.ir", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return IR(std::get<Indicator>(pp.at("p").at(0)), std::get<Indicator>(pp.at("b").at(0)),
                    static_cast<int>(sc.get<int64_t>("n")));
      });
}

void registerPart_LONGCROSS() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "LONGCROSS";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.longcross", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return LONGCROSS(std::get<Indicator>(pp.at("x").at(0)),
                                               std::get<Indicator>(pp.at("y").at(0)),
                                               static_cast<int>(sc.get<int64_t>("n")));
                          });
}

void registerPart_MAX() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MAX";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.max", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MAX(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

void registerPart_MIN() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MIN";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.min", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MIN(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

void registerPart_MOD() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MOD";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.mod", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MOD(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

void registerPart_NDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "NDAY";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart(
      "core.ind.nday", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return NDAY(std::get<Indicator>(pp.at("x").at(0)), std::get<Indicator>(pp.at("y").at(0)),
                      static_cast<int>(sc.get<int64_t>("n")));
      });
}

void registerPart_UPNDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "UPNDAY";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.upnday", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return UPNDAY(std::get<Indicator>(pp.at("ind").at(0)),
                                            static_cast<int>(sc.get<int64_t>("n")));
                          });
}

void registerIndicatorParts() {
    registerPart_ABS();
    registerPart_ACOS();
    registerPart_ADJ_FACTOR();
    registerPart_ADVANCE();
    registerPart_ADX();
    registerPart_ALIGN();
    registerPart_AMA();
    registerPart_ASIN();
    registerPart_ATAN();
    registerPart_ATR();
    registerPart_BACKSET();
    registerPart_BARSCOUNT();
    registerPart_BARSLAST();
    registerPart_BARSLASTCOUNT();
    registerPart_BARSSINCE();
    registerPart_CEILING();
    registerPart_COS();
    registerPart_COUNT();
    registerPart_DECLINE();
    registerPart_DEVSQ();
    registerPart_DIFF();
    registerPart_DROPNA();
    registerPart_EMA();
    registerPart_EVERY();
    registerPart_EXIST();
    registerPart_EXP();
    registerPart_FILTER();
    registerPart_FLOOR();
    registerPart_HHV();
    registerPart_HHVBARS();
    registerPart_INDEXO();
    registerPart_INTPART();
    registerPart_ISINF();
    registerPart_ISINFA();
    registerPart_ISLASTBAR();
    registerPart_ISLIMITDOWN();
    registerPart_ISLIMITUP();
    registerPart_ISNA();
    registerPart_JUMPDOWN();
    registerPart_JUMPUP();
    registerPart_KALMAN();
    registerPart_KDATA();
    registerPart_KURT();
    registerPart_LASTVALUE();
    registerPart_LIUTONGPAN();
    registerPart_LLV();
    registerPart_LLVBARS();
    registerPart_LN();
    registerPart_LOG();
    registerPart_MA();
    registerPart_MACD();
    registerPart_MDD();
    registerPart_MDD_CURRENT();
    registerPart_MRR();
    registerPart_NOT();
    registerPart_QUANTILE_TRUNC();
    registerPart_RECOVER_FORWARD();
    registerPart_REPLACE();
    registerPart_REVERSE();
    registerPart_ROC();
    registerPart_ROCP();
    registerPart_ROCR();
    registerPart_ROCR100();
    registerPart_ROUND();
    registerPart_ROUNDDOWN();
    registerPart_ROUNDUP();
    registerPart_RSRS_BETA();
    registerPart_RSRS_BULL();
    registerPart_SAFTYLOSS();
    registerPart_SGN();
    registerPart_SIN();
    registerPart_SKEW();
    registerPart_SLOPE();
    registerPart_SMA();
    registerPart_SQRT();
    registerPart_STDEV();
    registerPart_STDP();
    registerPart_STKTYPE();
    registerPart_SUM();
    registerPart_TAN();
    registerPart_DATE();
    registerPart_TIMELINE();
    registerPart_TIMELINEVOL();
    registerPart_TR();
    registerPart_TS_RANK();
    registerPart_VAR();
    registerPart_VARP();
    registerPart_WINNER();
    registerPart_WMA();
    registerPart_ZHBOND10();
    registerPart_ZONGGUBEN();
    registerPart_ZSCORE();
    registerPart_LAST();
    registerPart_RSI();
    registerPart_TURNOVER();
    registerPart_AD();
    registerPart_ADX2();
    registerPart_BARSLASTS();
    registerPart_BETA();
    registerPart_BLOCKSETNUM();
    registerPart_CODELIKE();
    registerPart_CONTEXT();
    registerPart_CORR();
    registerPart_COST();
    registerPart_COV();
    registerPart_CVAL();
    registerPart_CYCLE();
    registerPart_DISCARD();
    registerPart_DMA();
    registerPart_FACTOR();
    registerPart_FINANCE();
    registerPart_HSL();
    registerPart_IC();
    registerPart_INBLOCK();
    registerPart_INSUM();
    registerPart_NAMELIKE();
    registerPart_POW();
    registerPart_PRICELIST();
    registerPart_REF();
    registerPart_REFX();
    registerPart_RESULT();
    registerPart_SIGNED_POWER();
    registerPart_SLICE();
    registerPart_SPEARMAN();
    registerPart_SUMBARS();
    registerPart_VIGOR();
    registerPart_AVEDEV();
    registerPart_BETWEEN();
    registerPart_CROSS();
    registerPart_DOWNNDAY();
    registerPart_ICIR();
    registerPart_IR();
    registerPart_LONGCROSS();
    registerPart_MAX();
    registerPart_MIN();
    registerPart_MOD();
    registerPart_NDAY();
    registerPart_UPNDAY();
}

}  // namespace hku
