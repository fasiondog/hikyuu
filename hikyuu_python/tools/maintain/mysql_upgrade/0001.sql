CREATE TABLE `hku_base`.`version` (
	`version` INT(11) UNSIGNED NOT NULL
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB;
INSERT `hku_base`.`version` (version) VALUES (1);

INSERT INTO `hku_base`.`Market` (marketid,market,name,description,code,lastDate) VALUES (3,'OC_','okex','OKEX.com','btc_usd',19901219);
INSERT INTO `hku_base`.`Market` (marketid,market,name,description,code,lastDate) VALUES (4,'HB_','huobi','火币网','btc_usd',19901219);

INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (1, 'btc_usd', '比特币/美元', 9, 1, 20140724, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (2, 'btc_usd', '比特币/美元', 9, 1, 20170822, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (1, 'ltc_usd', '莱特币/美元', 9, 1, 20140724, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (2, 'ltc_usd', '莱特币/美元', 9, 1, 20170822, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (1, 'eth_usd', '以太坊/美元', 9, 1, 20140724, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (2, 'eth_usd', '以太坊/美元', 9, 1, 20170822, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (1, 'etc_usd', '以太经典/美元', 9, 1, 20140724, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (2, 'etc_usd', '以太经典/美元', 9, 1, 20170822, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (1, 'bch_usd', '比特现金/美元', 9, 1, 20140724, 99999999);
INSERT INTO `hku_base`.`stock` (`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) VALUES (2, 'bch_usd', '比特现金/美元', 9, 1, 20170822, 99999999);

INSERT INTO `hku_base`.`stocktypeinfo` (`id`, `type`, `precision`, `tick`, `tickValue`, `minTradeNumber`, `maxTradeNumber`, `description`) VALUES (9, 9, 2, 1, 0.0001, '1', '10000000000', 'btc_usd');
