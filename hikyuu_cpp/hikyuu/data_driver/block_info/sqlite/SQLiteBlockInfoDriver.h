/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-03
 *      Author: fasiondog
 */

#pragma once

#include "../../BlockInfoDriver.h"

namespace hku {

class SQLiteBlockInfoDriver : public BlockInfoDriver {
public:
    SQLiteBlockInfoDriver() : BlockInfoDriver("sqlite3"){};
    virtual ~SQLiteBlockInfoDriver();

    virtual void load() override;
    virtual bool _init() override;
    virtual Block getBlock(const string&, const string&) override;
    virtual BlockList getBlockList(const string& category) override;
    virtual BlockList getBlockList() override;

private:
    unordered_map<string, unordered_map<string, Block>> m_buffer;
};

}  // namespace hku
