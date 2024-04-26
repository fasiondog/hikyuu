CREATE TABLE IF NOT EXISTS `hku_base`.`HistoryFinanceField` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `name` VARCHAR(200) NOT NULL, PRIMARY KEY (`id`)
) COLLATE = 'utf8mb4_general_ci' ENGINE = MyISAM;

CREATE TABLE IF NOT EXISTS `hku_base`.`HistoryFinance` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `file_date` INT UNSIGNED NOT NULL, `market_code` VARCHAR(60) NOT NULL, `report_date` INT UNSIGNED NOT NULL, `values` BLOB NOT NULL, PRIMARY KEY (`id`), INDEX `ix1_on_history_finance` (`file_date`), INDEX `ix2_on_history_finance` (`market_code`, `report_date`)
) COLLATE = 'utf8mb4_general_ci' ENGINE = MyISAM;

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (1, "基本每股收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (2, "扣除非经常性损益每股收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (3, "每股未分配利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (4, "每股净资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (5, "每股资本公积金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (6, "净资产收益率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (7, "每股经营现金流量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (8, "资产负债表_货币资金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (9, "资产负债表_交易性金融资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (10, "资产负债表_应收票据");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (11, "资产负债表_应收账款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (12, "资产负债表_预付款项");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (13, "资产负债表_其他应收款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (14, "资产负债表_应收关联公司款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (15, "资产负债表_应收利息");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (16, "资产负债表_应收股利");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (17, "资产负债表_存货");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (18, "资产负债表_消耗性生物资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (19, "资产负债表_一年内到期的非流动资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (20, "资产负债表_其他流动资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (21, "资产负债表_流动资产合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (22, "资产负债表_可供出售金融资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (23, "资产负债表_持有至到期投资");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (24, "资产负债表_长期应收款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (25, "资产负债表_长期股权投资");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (26, "资产负债表_投资性房地产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (27, "资产负债表_固定资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (28, "资产负债表_在建工程");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (29, "资产负债表_工程物资");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (30, "资产负债表_固定资产清理");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (31, "资产负债表_生产性生物资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (32, "资产负债表_油气资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (33, "资产负债表_无形资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (34, "资产负债表_开发支出");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (35, "资产负债表_商誉");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (36, "资产负债表_长期待摊费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (37, "资产负债表_递延所得税资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (38, "资产负债表_其他非流动资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (39, "资产负债表_非流动资产合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (40, "资产负债表_资产总计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (41, "资产负债表_短期借款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (42, "资产负债表_交易性金融负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (43, "资产负债表_应付票据");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (44, "资产负债表_应付账款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (45, "资产负债表_预收款项");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (46, "资产负债表_应付职工薪酬");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (47, "资产负债表_应交税费");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (48, "资产负债表_应付利息");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (49, "资产负债表_应付股利");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (50, "资产负债表_其他应付款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (51, "资产负债表_应付关联公司款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (52, "资产负债表_一年内到期的非流动负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (53, "资产负债表_其他流动负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (54, "资产负债表_流动负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (55, "资产负债表_长期借款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (56, "资产负债表_应付债券");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (57, "资产负债表_长期应付款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (58, "资产负债表_专项应付款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (59, "资产负债表_预计负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (60, "资产负债表_递延所得税负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (61, "资产负债表_其他非流动负债");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (62, "资产负债表_非流动负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (63, "资产负债表_负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (64, "资产负债表_实收资本（或股本）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (65, "资产负债表_资本公积");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (66, "资产负债表_盈余公积");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (67, "资产负债表_库存股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (68, "资产负债表_未分配利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (69, "资产负债表_少数股东权益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (70, "资产负债表_外币报表折算价差");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (71, "资产负债表_非正常经营项目收益调整");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (72, "资产负债表_所有者权益（或股东权益）合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (73, "资产负债表_负债和所有者（或股东权益）合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (74, "利润表_营业收入");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (75, "利润表_营业成本");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (76, "利润表_营业税金及附加");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (77, "利润表_销售费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (78, "利润表_管理费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (79, "利润表_勘探费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (80, "利润表_财务费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (81, "利润表_资产减值损失");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (82, "利润表_公允价值变动净收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (83, "利润表_投资收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (84, "利润表_对联营企业和合营企业的投资收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (85, "利润表_影响营业利润的其他科目");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (86, "利润表_营业利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (87, "利润表_补贴收入");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (88, "利润表_营业外收入");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (89, "利润表_营业外支出");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (90, "利润表_非流动资产处置净损失");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (91, "利润表_影响利润总额的其他科目");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (92, "利润表_利润总额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (93, "利润表_所得税");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (94, "利润表_影响净利润的其他科目");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (95, "利润表_净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (96, "利润表_归属于母公司所有者的净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (97, "利润表_少数股东损益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (98, "现金流量表_销售商品、提供劳务收到的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (99, "现金流量表_收到的税费返还");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (100, "现金流量表_收到其他与经营活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (101, "现金流量表_经营活动现金流入小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (102, "现金流量表_购买商品、接受劳务支付的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (103, "现金流量表_支付给职工以及为职工支付的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (104, "现金流量表_支付的各项税费");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (105, "现金流量表_支付其他与经营活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (106, "现金流量表_经营活动现金流出小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (107, "现金流量表_经营活动产生的现金流量净额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (108, "现金流量表_收回投资收到的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (109, "现金流量表_取得投资收益收到的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        110, "现金流量表_处置固定资产、无形资产和其他长期资产收回的现金净额"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        111, "现金流量表_处置子公司及其他营业单位收到的现金净额"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (112, "现金流量表_收到其他与投资活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (113, "现金流量表_投资活动现金流入小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        114, "现金流量表_购建固定资产、无形资产和其他长期资产支付的现金"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (115, "现金流量表_投资支付的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        116, "现金流量表_取得子公司及其他营业单位支付的现金净额"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (117, "现金流量表_支付其他与投资活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (118, "现金流量表_投资活动现金流出小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (119, "现金流量表_投资活动产生的现金流量净额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (120, "现金流量表_吸收投资收到的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (121, "现金流量表_取得借款收到的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (122, "现金流量表_收到其他与筹资活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (123, "现金流量表_筹资活动现金流入小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (124, "现金流量表_偿还债务支付的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        125, "现金流量表_分配股利、利润或偿付利息支付的现金"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (126, "现金流量表_支付其他与筹资活动有关的现金");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (127, "现金流量表_筹资活动现金流出小计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (128, "现金流量表_筹资活动产生的现金流量净额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (129, "现金流量表_汇率变动对现金的影响");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (130, "现金流量表_其他原因对现金的影响");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (131, "现金流量表_现金及现金等价物净增加额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (132, "现金流量表_期初现金及现金等价物余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (133, "现金流量表_期末现金及现金等价物余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (134, "现金流量表_净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (135, "现金流量表_资产减值准备");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        136, "现金流量表_固定资产折旧、油气资产折耗、生产性生物资产折旧"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (137, "现金流量表_无形资产摊销");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (138, "现金流量表_长期待摊费用摊销");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        139, "现金流量表_处置固定资产、无形资产和其他长期资产的损失"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (140, "现金流量表_固定资产报废损失");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (141, "现金流量表_公允价值变动损失");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (142, "现金流量表_财务费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (143, "现金流量表_投资损失");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (144, "现金流量表_递延所得税资产减少");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (145, "现金流量表_递延所得税负债增加");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (146, "现金流量表_存货的减少");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (147, "现金流量表_经营性应收项目的减少");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (148, "现金流量表_经营性应付项目的增加");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (149, "现金流量表_其他");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (150, "现金流量表_经营活动产生的现金流量净额2");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (151, "现金流量表_债务转为资本");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (152, "现金流量表_一年内到期的可转换公司债券");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (153, "现金流量表_融资租入固定资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (154, "现金流量表_现金的期末余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (155, "现金流量表_现金的期初余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (156, "现金流量表_现金等价物的期末余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (157, "现金流量表_现金等价物的期初余额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (158, "现金流量表_现金及现金等价物净增加额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (159, "偿债能力_流动比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (160, "偿债能力_速动比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (161, "偿债能力_现金比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (162, "偿债能力_利息保障倍数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (163, "偿债能力_非流动负债比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (164, "偿债能力_流动负债比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (165, "偿债能力_现金到期债务比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (166, "偿债能力_有形资产净值债务率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (167, "偿债能力_权益乘数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (168, "偿债能力_股东的权益/负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (169, "偿债能力_有形资产/负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        170, "偿债能力_经营活动产生的现金流量净额/负债合计"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (171, "偿债能力_EBITDA/负债合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (172, "营运能力_应收帐款周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (173, "营运能力_存货周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (174, "营运能力_运营资金周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (175, "营运能力_总资产周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (176, "营运能力_固定资产周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (177, "营运能力_应收帐款周转天数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (178, "营运能力_存货周转天数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (179, "营运能力_流动资产周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (180, "营运能力_流动资产周转天数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (181, "营运能力_总资产周转天数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (182, "营运能力_股东权益周转率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (183, "成长能力_营业收入增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (184, "成长能力_净利润增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (185, "成长能力_净资产增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (186, "成长能力_固定资产增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (187, "成长能力_总资产增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (188, "成长能力_投资收益增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (189, "成长能力_营业利润增长率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (190, "成长能力_扣非每股收益同比");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (191, "成长能力_扣非净利润同比");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (192, "成长能力_暂无");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (193, "盈利能力_成本费用利润率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (194, "盈利能力_营业利润率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (195, "盈利能力_营业税金率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (196, "盈利能力_营业成本率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (197, "盈利能力_净资产收益率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (198, "盈利能力_投资收益率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (199, "盈利能力_销售净利率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (200, "盈利能力_总资产报酬率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (201, "盈利能力_净利润率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (202, "盈利能力_销售毛利率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (203, "盈利能力_三费比重");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (204, "盈利能力_管理费用率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (205, "盈利能力_财务费用率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (206, "盈利能力_扣除非经常性损益后的净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (207, "盈利能力_息税前利润(EBIT)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (208, "盈利能力_息税折旧摊销前利润(EBITDA)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (209, "盈利能力_EBITDA/营业总收入");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (210, "资本结构_资产负债率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (211, "资本结构_流动资产比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (212, "资本结构_货币资金比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (213, "资本结构_存货比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (214, "资本结构_固定资产比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (215, "资本结构_负债结构比");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (216, "资本结构_归属于母公司股东权益/全部投入资本");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (217, "资本结构_股东的权益/带息债务");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (218, "资本结构_有形资产/净债务");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (219, "现金能力_每股经营性现金流");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (220, "现金能力_营业收入现金含量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        221, "现金能力_经营活动产生的现金流量净额/经营活动净收益"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        222, "现金能力_销售商品提供劳务收到的现金/营业收入"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        223, "现金能力_经营活动产生的现金流量净额/营业收入"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (224, "现金能力_资本支出/折旧和摊销");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (225, "现金能力_每股现金流量净额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (226, "现金能力_经营净现金比率（短期债务）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (227, "现金能力_经营净现金比率（全部债务）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (228, "现金能力_经营活动现金净流量与净利润比率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (229, "现金能力_全部资产现金回收率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (230, "利润表_营业收入_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (231, "利润表_营业利润_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (232, "利润表_归属于母公司所有者的净利润_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (233, "利润表_扣除非经常性损益后的净利润_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        234, "现金流量表_经营活动产生的现金流量净额_单季度"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        235, "现金流量表_投资活动产生的现金流量净额_单季度"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        236, "现金流量表_筹资活动产生的现金流量净额_单季度"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (237, "现金流量表_现金及现金等价物净增加额_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (238, "股本股东_总股本");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (239, "股本股东_已上市流通A股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (240, "股本股东_已上市流通B股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (241, "股本股东_已上市流通H股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (242, "股本股东_股东人数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (243, "股本股东_第一大股东的持股数量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (244, "股本股东_十大流通股东持股数量合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (245, "股本股东_十大股东持股数量合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (246, "股本股东_机构总量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (247, "股本股东_机构持股总量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (248, "股本股东_QFII机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (249, "股本股东_QFII持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (250, "股本股东_券商机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (251, "股本股东_券商持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (252, "股本股东_保险机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (253, "股本股东_保险持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (254, "股本股东_基金机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (255, "股本股东_基金持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (256, "股本股东_社保机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (257, "股本股东_社保持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (258, "股本股东_私募机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (259, "股本股东_私募持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (260, "股本股东_财务公司机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (261, "股本股东_财务公司持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (262, "股本股东_年金机构数");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (263, "股本股东_年金持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (264, "股本股东_十大流通股东中持有A股合计");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (265, "股本股东_第一大流通股东持股量");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (266, "股本股东_自由流通股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (267, "股本股东_受限流通A股");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (268, "资产负债表_一般风险准备");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (269, "利润表_其他综合收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (270, "利润表_综合收益总额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (271, "资产负债表_归属于母公司股东权益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (272, "股本股东_银行机构数(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (273, "股本股东_银行持股量(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (274, "股本股东_一般法人机构数(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (275, "股本股东_一般法人持股量(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (276, "利润表_近一年净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (277, "股本股东_信托机构数(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (278, "股本股东_信托持股量(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (279, "股本股东_特殊法人机构数(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (280, "股本股东_特殊法人持股量(机构持股)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (281, "盈利能力_加权净资产收益率(每股指标)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (282, "利润表_扣非每股收益_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (283, "利润表_最近一年营业收入（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (284, "股本股东_国家队持股数量（万股）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (285, "业绩预告_本期净利润同比增幅下限%");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (286, "业绩预告_本期净利润同比增幅上限%");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (287, "业绩快报_归母净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (288, "业绩快报_扣非净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (289, "业绩快报_总资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (290, "业绩快报_净资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (291, "业绩快报_每股收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (292, "业绩快报_摊薄净资产收益率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (293, "业绩快报_加权净资产收益率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (294, "业绩快报_每股净资产");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (295, "资产负债表_应付票据及应付账款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (296, "资产负债表_应收票据及应收账款");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (297, "资产负债表_递延收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (298, "资产负债表_其他综合收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (299, "资产负债表_其他权益工具");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (300, "利润表_其他收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (301, "利润表_资产处置收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (302, "利润表_持续经营净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (303, "利润表_终止经营净利润");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (304, "利润表_研发费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (305, "利润表_利息费用");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (306, "利润表_利息收入");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (307, "现金流量表_近一年经营活动现金流净额");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (308, "现金流量表_近一年归母净利润（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (309, "现金流量表_近一年扣非净利润（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (310, "现金流量表_近一年现金净流量（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (311, "利润表_基本每股收益_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (312, "利润表_营业总收入（万元）_单季度");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (313, "业绩预告公告日期 ");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (314, "财报公告日期");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (315, "业绩快报公告日期");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (316, "现金流量表_近一年投资活动现金流净额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (317, "业绩预告_业绩预告-本期净利润下限（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (318, "业绩预告_业绩预告-本期净利润上限（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (319, "利润表_营业总收入TTM（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (320, "员工总数(人)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (321, "现金流量表_每股企业自由现金流");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (322, "现金流量表_每股股东自由现金流");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (323, "近一年营业利润（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (324, "净利润（单季度）(万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (325, "北上资金数（家）(机构持股）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (326, "北上资金持股量（股）(机构持股）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (327, "有息负债率");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (328, "营业成本（单季度）(万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (329, "投入资本回报率（ROIC）(获利能力分析)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (330, "业绩快报-营业收入（本期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (331, "业绩快报-营业收入（上期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (332, "业绩快报-营业利润（本期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (333, "业绩快报-营业利润（上期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (334, "业绩快报-利润总额（本期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (335, "业绩快报-利润总额（上期）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (336, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (337, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (338, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (339, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (340, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (341, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (342, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (343, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (344, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (345, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (346, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (347, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (348, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (349, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (350, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (351, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (352, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (353, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (354, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (355, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (356, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (357, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (358, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (359, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (360, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (361, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (362, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (363, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (364, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (365, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (366, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (367, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (368, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (369, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (370, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (371, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (372, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (373, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (374, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (375, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (376, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (377, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (378, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (379, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (380, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (381, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (382, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (383, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (384, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (385, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (386, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (387, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (388, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (389, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (390, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (391, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (392, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (393, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (394, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (395, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (396, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (397, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (398, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (399, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (400, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (401, "资产负债表_专项储备（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (402, "资产负债表_结算备付金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (403, "资产负债表_拆出资金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        404, "资产负债表_发放贷款及垫款（万元）(流动资产科目)"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (405, "资产负债表_衍生金融资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (406, "资产负债表_应收保费（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (407, "资产负债表_应收分保账款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (408, "资产负债表_应收分保合同准备金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (409, "资产负债表_买入返售金融资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (410, "资产负债表_划分为持有待售的资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        411, "资产负债表_发放贷款及垫款（万元）(非流动资产科目)"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (412, "资产负债表_向中央银行借款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (413, "资产负债表_吸收存款及同业存放（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (414, "资产负债表_拆入资金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (415, "资产负债表_衍生金融负债（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (416, "资产负债表_卖出回购金融资产款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (417, "资产负债表_应付手续费及佣金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (418, "资产负债表_应付分保账款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (419, "资产负债表_保险合同准备金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (420, "资产负债表_代理买卖证券款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (421, "资产负债表_代理承销证券款（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (422, "资产负债表_划分为持有待售的负债（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (423, "资产负债表_预计负债（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (424, "资产负债表_递延收益（万元）（流动负债科目）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        425, "资产负债表_其中:优先股（万元）(非流动负债科目)"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (426, "资产负债表_永续债（万元）(非流动负债科目)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (427, "资产负债表_长期应付职工薪酬（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        428, "资产负债表_其中:优先股（万元）(所有者权益科目)"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (429, "资产负债表_永续债（万元）(所有者权益科目)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (430, "资产负债表_债权投资（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (431, "资产负债表_其他债权投资（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (432, "资产负债表_其他权益工具投资（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (433, "资产负债表_其他非流动金融资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (434, "资产负债表_合同负债（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (435, "资产负债表_合同资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (436, "资产负债表_其他资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (437, "资产负债表_应收款项融资（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (438, "资产负债表_使用权资产（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (439, "资产负债表_租赁负债（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (440, "发放贷款及垫款(万元)");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (441, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (442, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (443, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (444, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (445, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (446, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (447, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (448, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (449, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (450, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (451, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (452, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (453, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (454, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (455, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (456, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (457, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (458, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (459, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (460, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (461, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (462, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (463, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (464, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (465, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (466, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (467, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (468, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (469, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (470, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (471, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (472, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (473, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (474, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (475, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (476, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (477, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (478, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (479, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (480, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (481, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (482, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (483, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (484, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (485, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (486, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (487, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (488, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (489, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (490, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (491, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (492, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (493, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (494, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (495, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (496, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (497, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (498, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (499, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (500, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (501, "利润表_稀释每股收益");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (502, "利润表_营业总收入（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (503, "利润表_汇兑收益（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (504, "利润表_其中:归属于母公司综合收益（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (505, "利润表_其中:归属于少数股东综合收益（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (506, "利润表_利息收入（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (507, "利润表_已赚保费（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (508, "利润表_手续费及佣金收入（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (509, "利润表_利息支出（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (510, "利润表_手续费及佣金支出（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (511, "利润表_退保金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (512, "利润表_赔付支出净额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (513, "利润表_提取保险合同准备金净额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (514, "利润表_保单红利支出（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (515, "利润表_分保费用（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (516, "利润表_其中:非流动资产处置利得（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (517, "利润表_信用减值损失（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (518, "利润表_净敞口套期收益（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (519, "利润表_营业总成本（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (520, "利润表_信用减值损失（万元、2019格式）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (521, "利润表_资产减值损失（万元、2019格式）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (522, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (523, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (524, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (525, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (526, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (527, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (528, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (529, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (530, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (531, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (532, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (533, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (534, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (535, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (536, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (537, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (538, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (539, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (540, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (541, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (542, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (543, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (544, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (545, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (546, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (547, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (548, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (549, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (550, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (551, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (552, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (553, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (554, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (555, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (556, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (557, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (558, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (559, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (560, "未知");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (561, "现金流量表_加:其他原因对现金的影响（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        562, "现金流量表_客户存款和同业存放款项净增加额（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (563, "现金流量表_向中央银行借款净增加额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        564, "现金流量表_向其他金融机构拆入资金净增加额（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        565, "现金流量表_收到原保险合同保费取得的现金（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (566, "现金流量表_收到再保险业务现金净额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (567, "现金流量表_保户储金及投资款净增加额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        568, "现金流量表_处置以公允价值计量且其变动计入当期损益的金融资产净增加额（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        569, "现金流量表_收取利息、手续费及佣金的现金（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (570, "现金流量表_拆入资金净增加额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (571, "现金流量表_回购业务资金净增加额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (572, "现金流量表_客户贷款及垫款净增加额（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        573, "现金流量表_存放中央银行和同业款项净增加额（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        574, "现金流量表_支付原保险合同赔付款项的现金（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        575, "现金流量表_支付利息、手续费及佣金的现金（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (576, "现金流量表_支付保单红利的现金（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        577, "现金流量表_其中:子公司吸收少数股东投资收到的现金（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (
        578, "现金流量表_其中:子公司支付给少数股东的股利、利润（万元）"
    );

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (579, "现金流量表_投资性房地产的折旧及摊销（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (580, "现金流量表_信用减值损失（万元）");

INSERT INTO
    `hku_base`.`HistoryFinanceField` (`id`, `name`)
VALUES (581, "使用权资产折旧（万元）");

UPDATE `hku_base`.`version` set `version` = 13;